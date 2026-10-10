/* QEMU-targeted FDT RAM discovery and bitmap-backed 4 KiB physical page allocator. */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#define FDT_MAGIC 0xd00dfeedu
#define FDT_MAX_SIZE (16u*1024u*1024u)
#define FDT_MAX_DEPTH 24
#define PAGE_SIZE 4096ULL
#define MAX_PAGES 262144u
#define BITMAP_SIZE (MAX_PAGES/8u)
extern uint8_t __kernel_end[];
static uint8_t bitmap[BITMAP_SIZE];
static uint64_t ram_base,ram_size,fdt_addr,pool_start,pool_end;
static uint32_t fdt_size,pool_pages,free_count;
static bool ready;
static void put(const char*s){uart_write_external(s);}
static void dec(uint64_t v){char b[21];size_t n=0;if(!v){put("0");return;}while(v&&n<21){b[n++]=(char)('0'+v%10);v/=10;}while(n){char x[2]={b[--n],0};put(x);}}
static bool eq(const char*a,const char*b){while(*a&&*b&&*a==*b){a++;b++;}return !*a&&!*b;}
static uint32_t be32(const uint8_t*p){return((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static uint64_t read_cells(const uint8_t*p,uint32_t n){uint64_t v=0;if(n>2)return 0;for(uint32_t i=0;i<n;i++)v=(v<<32)|be32(p+i*4u);return v;}
static uint64_t align_up(uint64_t v){return(v+PAGE_SIZE-1u)&~(PAGE_SIZE-1u);}
static uint64_t align_down(uint64_t v){return v&~(PAGE_SIZE-1u);}
static void copy_text(char*d,size_t cap,const char*s,uint32_t len){if(!cap)return;size_t i=0;while(i+1<cap&&i<len&&s[i]){d[i]=s[i];i++;}d[i]=0;}
struct fdt_node{char name[64],type[16];uint32_t parent_ac,parent_sc,ac,sc;uint64_t base,size;bool has_reg;};
static bool parse_fdt(uint64_t addr){
    if(!addr)return false;
    const uint8_t*b=(const uint8_t*)(uintptr_t)addr;
    if(be32(b)!=FDT_MAGIC)return false;
    uint32_t total=be32(b+4),off_struct=be32(b+8),off_strings=be32(b+12);
    uint32_t strings_size=be32(b+32),struct_size=be32(b+36);
    if(total<40||total>FDT_MAX_SIZE||off_struct>total||struct_size>total-off_struct||off_strings>total||strings_size>total-off_strings)return false;
    const uint8_t*p=b+off_struct,*end=p+struct_size;
    const char*strings=(const char*)(b+off_strings);
    struct fdt_node stack[FDT_MAX_DEPTH];uint32_t depth=0;bool found=false;
    while(p+4<=end){
        uint32_t token=be32(p);p+=4;
        if(token==1u){
            if(depth>=FDT_MAX_DEPTH)return false;
            struct fdt_node*n=&stack[depth];
            for(size_t i=0;i<sizeof(*n);i++)((uint8_t*)n)[i]=0;
            n->parent_ac=depth?stack[depth-1].ac:2u;
            n->parent_sc=depth?stack[depth-1].sc:1u;
            n->ac=n->parent_ac;n->sc=n->parent_sc;
            size_t len=0;while(p+len<end&&p[len])len++;
            if(p+len>=end)return false;
            copy_text(n->name,sizeof(n->name),(const char*)p,(uint32_t)len);
            p+=(len+4u)&~3u;depth++;
        }else if(token==2u){
            if(!depth)return false;
            struct fdt_node*n=&stack[depth-1];
            bool named=n->name[0]=='m'&&n->name[1]=='e'&&n->name[2]=='m'&&n->name[3]=='o'&&n->name[4]=='r'&&n->name[5]=='y'&&n->name[6]=='@';
            if(!found&&(eq(n->type,"memory")||named)&&n->has_reg&&n->size){ram_base=n->base;ram_size=n->size;found=true;}
            depth--;
        }else if(token==3u){
            if(!depth||p+8>end)return false;
            uint32_t len=be32(p),nameoff=be32(p+4);p+=8;
            if(len>(uint32_t)(end-p)||nameoff>=strings_size)return false;
            const char*name=strings+nameoff;size_t rem=strings_size-nameoff,nlen=0;
            while(nlen<rem&&name[nlen])nlen++;
            if(nlen==rem)return false;
            struct fdt_node*n=&stack[depth-1];
            if(eq(name,"#address-cells")&&len>=4){uint32_t v=be32(p);if(v&&v<=2)n->ac=v;}
            else if(eq(name,"#size-cells")&&len>=4){uint32_t v=be32(p);if(v<=2)n->sc=v;}
            else if(eq(name,"device_type"))copy_text(n->type,sizeof(n->type),(const char*)p,len);
            else if(eq(name,"reg")){uint32_t count=n->parent_ac+n->parent_sc;if(count&&count<=4&&len>=count*4u){n->base=read_cells(p,n->parent_ac);n->size=read_cells(p+n->parent_ac*4u,n->parent_sc);n->has_reg=n->size!=0;}}
            p+=(len+3u)&~3u;
        }else if(token==4u)continue;
        else if(token==9u)break;
        else return false;
    }
    if(found){fdt_addr=addr;fdt_size=total;}
    return found;
}
static void set_used(uint32_t i,bool value){uint8_t mask=(uint8_t)(1u<<(i&7u));if(value)bitmap[i>>3]|=mask;else bitmap[i>>3]&=(uint8_t)~mask;}
static bool is_used(uint32_t i){return(bitmap[i>>3]&(uint8_t)(1u<<(i&7u)))!=0;}
void memory_init(uint64_t fdt){
    for(uint32_t i=0;i<BITMAP_SIZE;i++)bitmap[i]=0xffu;
    if(!parse_fdt(fdt))return;
    uint64_t end=ram_base+ram_size;
    if(end<ram_base)return;
    uint64_t start=align_up(ram_base),kernel_end=align_up((uint64_t)(uintptr_t)__kernel_end);
    if(kernel_end>start)start=kernel_end;
    end=align_down(end);
    if(end<=start)return;
    uint64_t count=(end-start)/PAGE_SIZE;
    if(count>MAX_PAGES)count=MAX_PAGES;
    pool_start=start;pool_pages=(uint32_t)count;pool_end=start+count*PAGE_SIZE;
    for(uint32_t i=0;i<pool_pages;i++){
        uint64_t page_start=start+(uint64_t)i*PAGE_SIZE,page_end=page_start+PAGE_SIZE;
        bool reserved=page_start<align_up(fdt_addr+fdt_size)&&page_end>align_down(fdt_addr);
        set_used(i,reserved);
        if(!reserved)free_count++;
    }
    ready=free_count>0;
}
bool memory_is_ready(void){return ready;}
static uint64_t allocate_page(void){
    if(!ready)return 0;
    for(uint32_t i=0;i<pool_pages;i++)if(!is_used(i)){set_used(i,true);free_count--;return pool_start+(uint64_t)i*PAGE_SIZE;}
    return 0;
}
static bool release_page(uint64_t address){
    if(!ready||(address&(PAGE_SIZE-1u))||address<pool_start||address>=pool_end)return false;
    uint32_t i=(uint32_t)((address-pool_start)/PAGE_SIZE);
    if(!is_used(i))return false;
    set_used(i,false);free_count++;return true;
}
bool memory_self_test(void){
    if(!ready)return false;
    uint32_t before=free_count;uint64_t first=allocate_page(),second=allocate_page();
    bool valid=first&&second&&first!=second&&!(first&(PAGE_SIZE-1u))&&!(second&(PAGE_SIZE-1u));
    bool freed_first=first?release_page(first):false,freed_second=second?release_page(second):false;
    return valid&&freed_first&&freed_second&&free_count==before;
}
void memory_print_info(void){
    put("\nDedus physical memory report\n");
    if(!ram_size){put("  Device-tree RAM data unavailable.\n");return;}
    put("  FDT address: ");uart_hex_external(fdt_addr);
    put("\n  RAM base: ");uart_hex_external(ram_base);
    put("\n  RAM size: ");dec(ram_size/(1024u*1024u));put(" MiB\n");
    if(!ready){put("  Page allocator: unavailable\n");return;}
    put("  Managed start: ");uart_hex_external(pool_start);
    put("\n  Managed pages: ");dec(pool_pages);
    put("\n  Free 4 KiB pages: ");dec(free_count);put("\n");
}
