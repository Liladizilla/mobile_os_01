// Adizilla OS - Applications Module
// Terminal, File Manager, and Settings apps

// ==================== TERMINAL ====================
const Terminal = {
  window: null,
  output: null,
  input: null,
  prompt: null,
  commandHistory: [],
  historyIndex: -1,
  cwd: '/home/root',
  
  init(windowEl) {
    this.window = windowEl;
    this.output = windowEl.querySelector('#terminal-output');
    this.input = windowEl.querySelector('#terminal-input');
    this.prompt = windowEl.querySelector('#terminal-prompt');
    
    // Setup input handler
    this.input.addEventListener('keydown', (e) => this.handleInput(e));
    
    // Focus input on click
    windowEl.querySelector('.terminal').addEventListener('click', () => {
      this.input.focus();
    });
    
    // Initial message
    this.printWelcome();
  },
  
  printWelcome() {
    this.print('╔═══════════════════════════════════════════════════════════╗', 'info');
    this.print('║  Welcome to Adizilla Mobile OS v1.0.0                  ║', 'info');
    this.print('║  Built with ❤️ in Nairobi, Kenya 🇰🇪                    ║', 'info');
    this.print('╚═══════════════════════════════════════════════════════════╝', 'info');
    this.print('');
    this.print('Type "help" for available commands.', 'info');
    this.print('');
  },
  
  handleInput(e) {
    if (e.key === 'Enter') {
      const command = this.input.value;
      this.input.value = '';
      this.executeCommand(command);
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      this.navigateHistory(-1);
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      this.navigateHistory(1);
    } else if (e.key === 'Tab') {
      e.preventDefault();
      this.handleAutocomplete();
    }
  },
  
  navigateHistory(direction) {
    const newIndex = this.historyIndex + direction;
    
    if (newIndex >= 0 && newIndex < this.commandHistory.length) {
      this.historyIndex = newIndex;
      this.input.value = this.commandHistory[this.commandHistory.length - 1 - newIndex];
    } else if (newIndex < 0) {
      this.historyIndex = -1;
      this.input.value = '';
    }
  },
  
  async handleAutocomplete() {
    const input = this.input.value;
    const parts = input.split(/\s+/);
    const lastPart = parts[parts.length - 1];
    
    // Simple autocomplete - check against known commands
    const commands = ['ls', 'cd', 'pwd', 'cat', 'mkdir', 'touch', 'rm', 'cp', 'mv', 
                      'ps', 'top', 'kill', 'ping', 'ifconfig', 'clear', 'help', 
                      'whoami', 'date', 'uname', 'echo', 'history', 'exit', 'logout'];
    
    const matches = commands.filter(cmd => cmd.startsWith(lastPart));
    
    if (matches.length === 1) {
      parts[parts.length - 1] = matches[0];
      this.input.value = parts.join(' ');
    } else if (matches.length > 1) {
      this.print(`${lastPart}${matches.join('  ')}`, 'info');
    }
  },
  
  async executeCommand(command) {
    // Print command
    this.print(`${this.getPromptText()}${command}`, 'prompt');
    
    if (!command.trim()) {
      return;
    }
    
    // Add to history
    this.commandHistory.push(command);
    this.historyIndex = -1;
    
    // Parse command
    const parts = command.trim().split(/\s+/);
    const cmd = parts[0];
    const args = parts.slice(1);
    
    try {
      const result = await AdzOS.executeCommand(command);
      
      if (result.output === '__CLEAR__') {
        this.output.innerHTML = '';
      } else if (result.output) {
        this.print(result.output, result.exitCode === 0 ? 'success' : 'error');
      }
      
      // Update cwd if changed
      if (result.cwd && result.cwd !== this.cwd) {
        this.cwd = result.cwd;
        this.updatePrompt();
      }
    } catch (error) {
      this.print(`Error: ${error.message}`, 'error');
    }
  },
  
  getPromptText() {
    const user = AdzOS.state.currentUser?.username || 'guest';
    return `${user}@adizilla:${this.cwd}$ `;
  },
  
  updatePrompt() {
    this.prompt.textContent = this.getPromptText();
  },
  
  print(text, type = '') {
    const line = document.createElement('div');
    line.className = `terminal-line ${type}`;
    line.textContent = text;
    this.output.appendChild(line);
    this.output.scrollTop = this.output.scrollHeight;
  }
};

// ==================== FILE MANAGER ====================
const FileManager = {
  window: null,
  currentPath: '/home/root',
  fileList: null,
  pathDisplay: null,
  
  init(windowEl) {
    this.window = windowEl;
    this.fileList = windowEl.querySelector('#file-list');
    this.pathDisplay = windowEl.querySelector('#file-path');
    
    // Setup button handlers
    windowEl.querySelector('#btn-up').addEventListener('click', () => this.navigateUp());
    windowEl.querySelector('#btn-home').addEventListener('click', () => this.navigateHome());
    windowEl.querySelector('#btn-mkdir').addEventListener('click', () => this.createFolder());
    
    // Initial load
    this.refresh();
  },
  
  async refresh() {
    try {
      const files = await AdzOS.listFiles(this.currentPath);
      this.render(files);
      this.pathDisplay.textContent = this.currentPath;
    } catch (error) {
      console.error('Error loading files:', error);
    }
  },
  
  render(files) {
    this.fileList.innerHTML = '';
    
    // Add parent directory if not at root
    if (this.currentPath !== '/') {
      const item = this.createFileItem('..', 'directory', 'Parent Directory');
      item.addEventListener('click', () => this.navigateUp());
      this.fileList.appendChild(item);
    }
    
    // Add files and directories
    for (const [name, info] of Object.entries(files)) {
      if (name === this.currentPath) continue;
      
      const isDir = info.type === 'dir';
      const item = this.createFileItem(name, info.type, name);
      
      item.addEventListener('click', () => {
        if (isDir) {
          this.navigate(name);
        } else {
          this.openFile(name);
        }
      });
      
      this.fileList.appendChild(item);
    }
  },
  
  createFileItem(name, type, displayName) {
    const item = document.createElement('div');
    item.className = `file-item ${type}`;
    
    const icon = document.createElement('span');
    icon.className = 'file-icon';
    icon.textContent = type === 'directory' ? '📁' : '📄';
    
    const fileName = document.createElement('span');
    fileName.className = 'file-name';
    fileName.textContent = displayName;
    
    item.appendChild(icon);
    item.appendChild(fileName);
    
    return item;
  },
  
  async navigate(folder) {
    const newPath = this.currentPath === '/' 
      ? '/' + folder 
      : this.currentPath + '/' + folder;
    
    this.currentPath = newPath;
    await this.refresh();
  },
  
  navigateUp() {
    const parts = this.currentPath.split('/').filter(Boolean);
    parts.pop();
    this.currentPath = '/' + parts.join('/') || '/';
    this.refresh();
  },
  
  navigateHome() {
    this.currentPath = '/home/root';
    this.refresh();
  },
  
  async openFile(name) {
    const path = this.currentPath === '/' ? '/' + name : this.currentPath + '/' + name;
    const result = await AdzOS.readFile(path);
    
    if (result.content !== undefined) {
      alert(result.content);
    } else {
      alert('Error reading file: ' + result.error);
    }
  },
  
  async createFolder() {
    const name = prompt('Enter folder name:');
    if (name) {
      const path = this.currentPath === '/' ? '/' + name : this.currentPath + '/' + name;
      await AdzOS.createDir(path);
      await this.refresh();
    }
  }
};

// ==================== SETTINGS ====================
const Settings = {
  window: null,
  
  init(windowEl) {
    this.window = windowEl;
    
    // Load current settings
    this.loadSettings();
    
    // Setup handlers
    windowEl.querySelector('#setting-theme').addEventListener('change', (e) => {
      AdzOS.setTheme(e.target.value);
    });
    
    windowEl.querySelector('#btn-logout').addEventListener('click', () => {
      AdzOS.logout();
    });
    
    // Update uptime periodically
    this.updateUptime();
    setInterval(() => this.updateUptime(), 1000);
  },
  
  async loadSettings() {
    const user = AdzOS.state.currentUser;
    const system = await API.system();
    
    this.window.querySelector('#setting-hostname').textContent = 'adizilla';
    this.window.querySelector('#setting-uptime').textContent = AdzOS.getUptime();
    this.window.querySelector('#setting-user').textContent = user?.username || 'guest';
    this.window.querySelector('#setting-theme').value = AdzOS.state.theme;
  },
  
  updateUptime() {
    const uptimeEl = this.window.querySelector('#setting-uptime');
    if (uptimeEl) {
      uptimeEl.textContent = AdzOS.getUptime();
    }
  }
};

// ==================== INITIALIZATION ====================
document.addEventListener('DOMContentLoaded', async () => {
  // Initialize OS
  await AdzOS.init();
  
  // Initialize UI
  UI.init();
  
  // Setup login form
  const loginForm = document.getElementById('login-form');
  loginForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    
    const success = await AdzOS.login(username, password);
    
    if (!success) {
      alert('Login failed. Please check your credentials.');
    }
  });
});

/* Export for global access */
window.Terminal = Terminal;
window.FileManager = FileManager;
window.Settings = Settings;
