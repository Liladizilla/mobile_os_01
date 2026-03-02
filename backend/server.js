// Adizilla Mobile OS - Backend Server
// Built by Lila, Nairobi 🇰🇪

const express = require('express');
const cors = require('cors');
const path = require('path');
const fs = require('fs');
const { v4: uuidv4 } = require('uuid');

// Initialize Express
const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, '../frontend')));

// In-memory storage for OS state
const osState = {
  uptime: Date.now(),
  users: [
    { id: '1', username: 'root', password: 'root', role: 'root', active: true },
    { id: '2', username: 'admin', password: 'admin', role: 'admin', active: false },
    { id: '3', username: 'guest', password: 'guest', role: 'guest', active: false }
  ],
  processes: [
    { pid: 1, name: 'kernel', status: 'running', cpu: 5, mem: 128, user: 'root' },
    { pid: 2, name: 'init', status: 'running', cpu: 1, mem: 64, user: 'root' },
    { pid: 3, name: 'shell', status: 'running', cpu: 2, mem: 32, user: 'root' }
  ],
  filesystem: {
    '/': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/home': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/home/root': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/home/root/files': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/etc': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/var': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/tmp': { type: 'dir', permissions: 'rwxrwxrwx', owner: 'root' },
    '/bin': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' },
    '/usr': { type: 'dir', permissions: 'rwxr-xr-x', owner: 'root' }
  },
  fileContents: {
    '/home/root/welcome.txt': 'Welcome to Adizilla OS!\nBuilt by Lila in Nairobi 🇰🇪\n',
    '/etc/hostname': 'adizilla',
    '/etc/os-info': 'Adizilla Mobile OS v1.0.0\nBuilt with ❤️ in Kenya\n'
  },
  network: {
    interfaces: [
      { name: 'lo', ip: '127.0.0.1', status: 'up' },
      { name: 'eth0', ip: '192.168.1.100', status: 'up' },
      { name: 'wlan0', ip: '192.168.1.101', status: 'down' }
    ],
    connections: []
  },
  sessions: {},
  logs: [],
  aliases: {
    'll': 'ls -la',
    'la': 'ls -a',
    'l': 'ls -l',
    '..': 'cd ..',
    '~': 'cd ~'
  },
  commandHistory: [],
  currentUser: null
};

// System logging
function log(action, details) {
  const entry = {
    timestamp: new Date().toISOString(),
    action,
    details,
    user: osState.currentUser?.username || 'system'
  };
  osState.logs.unshift(entry);
  if (osState.logs.length > 1000) osState.logs.pop();
}

// ==================== ROUTES ====================

// Health check
app.get('/api/health', (req, res) => {
  res.json({ 
    status: 'online', 
    uptime: Date.now() - osState.uptime,
    version: '1.0.0'
  });
});

// System info
app.get('/api/system', (req, res) => {
  res.json({
    hostname: 'adizilla',
    platform: 'Adizilla Mobile OS',
    arch: 'arm64',
    cpuCount: 4,
    memory: { total: 8192, free: 4096 },
    uptime: Date.now() - osState.uptime
  });
});

// Process management
app.get('/api/processes', (req, res) => {
  res.json(osState.processes);
});

app.post('/api/processes', (req, res) => {
  const { name, command } = req.body;
  const pid = osState.processes.length + 1;
  const proc = {
    pid,
    name: name || command.split(' ')[0],
    status: 'running',
    cpu: Math.floor(Math.random() * 10),
    mem: Math.floor(Math.random() * 128) + 32,
    user: osState.currentUser?.username || 'root'
  };
  osState.processes.push(proc);
  log('process_start', { pid, name: proc.name });
  res.json(proc);
});

app.delete('/api/processes/:pid', (req, res) => {
  const pid = parseInt(req.params.pid);
  const idx = osState.processes.findIndex(p => p.pid === pid);
  if (idx > -1 && pid > 3) { // Don't kill kernel processes
    const proc = osState.processes.splice(idx, 1)[0];
    log('process_kill', { pid, name: proc.name });
    res.json({ success: true });
  } else {
    res.status(400).json({ error: 'Cannot kill system process' });
  }
});

// File system
app.get('/api/fs', (req, res) => {
  const { path: fsPath } = req.query;
  const result = {};
  
  for (const [key, val] of Object.entries(osState.filesystem)) {
    if (key.startsWith(fsPath || '/')) {
      const relative = key.slice((fsPath || '/').length).split('/')[1];
      if (relative && !result['/' + relative]) {
        result['/' + relative] = val;
      }
    }
  }
  
  res.json(result);
});

app.get('/api/fs/read', (req, res) => {
  const { path: filePath } = req.query;
  const content = osState.fileContents[filePath];
  if (content !== undefined) {
    log('file_read', { path: filePath });
    res.json({ content });
  } else {
    res.status(404).json({ error: 'File not found' });
  }
});

app.post('/api/fs/write', (req, res) => {
  const { path: filePath, content } = req.body;
  osState.fileContents[filePath] = content;
  if (!osState.filesystem[filePath]) {
    osState.filesystem[filePath] = { type: 'file', permissions: 'rw-r--r--', owner: osState.currentUser?.username || 'root' };
  }
  log('file_write', { path: filePath });
  res.json({ success: true });
});

app.post('/api/fs/delete', (req, res) => {
  const { path: filePath } = req.body;
  delete osState.fileContents[filePath];
  delete osState.filesystem[filePath];
  log('file_delete', { path: filePath });
  res.json({ success: true });
});

app.post('/api/fs/mkdir', (req, res) => {
  const { path: dirPath } = req.body;
  osState.filesystem[dirPath] = { type: 'dir', permissions: 'rwxr-xr-x', owner: osState.currentUser?.username || 'root' };
  log('dir_create', { path: dirPath });
  res.json({ success: true });
});

// User management
app.get('/api/users', (req, res) => {
  res.json(osState.users.map(u => ({ ...u, password: undefined })));
});

app.post('/api/auth/login', (req, res) => {
  const { username, password } = req.body;
  const user = osState.users.find(u => u.username === username && u.password === password);
  if (user) {
    const sessionId = uuidv4();
    osState.sessions[sessionId] = { userId: user.id, username: user.username };
    user.active = true;
    osState.currentUser = user;
    log('user_login', { username });
    res.json({ success: true, sessionId, user: { ...user, password: undefined } });
  } else {
    res.status(401).json({ error: 'Invalid credentials' });
  }
});

app.post('/api/auth/logout', (req, res) => {
  if (osState.currentUser) {
    log('user_logout', { username: osState.currentUser.username });
    osState.currentUser.active = false;
    osState.currentUser = null;
  }
  res.json({ success: true });
});

app.get('/api/auth/me', (req, res) => {
  if (osState.currentUser) {
    res.json({ ...osState.currentUser, password: undefined });
  } else {
    res.status(401).json({ error: 'Not logged in' });
  }
});

// Network
app.get('/api/network', (req, res) => {
  res.json(osState.network);
});

app.post('/api/network/ping', (req, res) => {
  const { host } = req.body;
  // Simulate ping
  const success = Math.random() > 0.2;
  log('network_ping', { host, success });
  res.json({ host, success, time: Math.floor(Math.random() * 100) + 'ms' });
});

// Command history and aliases
app.get('/api/history', (req, res) => {
  res.json(osState.commandHistory.slice(-50));
});

app.post('/api/history', (req, res) => {
  const { command } = req.body;
  osState.commandHistory.push(command);
  if (osState.commandHistory.length > 100) osState.commandHistory.shift();
  res.json({ success: true });
});

app.get('/api/aliases', (req, res) => {
  res.json(osState.aliases);
});

app.post('/api/aliases', (req, res) => {
  const { alias, command } = req.body;
  osState.aliases[alias] = command;
  res.json({ success: true });
});

// Logs
app.get('/api/logs', (req, res) => {
  const { limit = 50 } = req.query;
  res.json(osState.logs.slice(0, parseInt(limit)));
});

// Terminal command execution
app.post('/api/terminal/exec', (req, res) => {
  const { command, cwd = '/home/root' } = req.body;
  log('command_exec', { command, cwd, user: osState.currentUser?.username });
  
  const parts = command.trim().split(/\s+/);
  const cmd = parts[0];
  const args = parts.slice(1);
  
  let output = '';
  let exitCode = 0;
  
  switch (cmd) {
    case 'ls':
      output = Object.keys(osState.filesystem)
        .filter(p => p.startsWith(cwd) && p !== cwd)
        .map(p => {
          const name = p.replace(cwd, '').split('/')[1];
          const item = osState.filesystem[p];
          return item.type === 'dir' ? `\x1b[34m${name}/\x1b[0m` : name;
        })
        .filter((v, i, a) => a.indexOf(v) === i)
        .join('  ');
      break;
      
    case 'pwd':
      output = cwd;
      break;
      
    case 'whoami':
      output = osState.currentUser?.username || 'guest';
      break;
      
    case 'uname':
      output = args.includes('-a') ? 'Adizilla 1.0.0 arm64 GNU/Linux' : 'Adizilla';
      break;
      
    case 'date':
      output = new Date().toString();
      break;
      
    case 'echo':
      output = args.join(' ');
      break;
      
    case 'cat':
      if (args[0]) {
        const content = osState.fileContents[args[0]];
        output = content || '';
      }
      break;
      
    case 'mkdir':
      if (args[0]) {
        const dirPath = cwd === '/' ? '/' + args[0] : cwd + '/' + args[0];
        osState.filesystem[dirPath] = { type: 'dir', permissions: 'rwxr-xr-x', owner: osState.currentUser?.username || 'root' };
        output = '';
      }
      break;
      
    case 'touch':
      if (args[0]) {
        const filePath = cwd === '/' ? '/' + args[0] : cwd + '/' + args[0];
        if (!osState.filesystem[filePath]) {
          osState.filesystem[filePath] = { type: 'file', permissions: 'rw-r--r--', owner: osState.currentUser?.username || 'root' };
          osState.fileContents[filePath] = '';
        }
        output = '';
      }
      break;
      
    case 'rm':
      if (args[0]) {
        const filePath = cwd === '/' ? '/' + args[0] : cwd + '/' + args[0];
        delete osState.filesystem[filePath];
        delete osState.fileContents[filePath];
        output = '';
      }
      break;
      
    case 'ps':
      output = osState.processes
        .map(p => `${p.pid}  ${p.user}  ${p.cpu}%  ${p.mem}MB  ${p.status}  ${p.name}`)
        .join('\n');
      break;
      
    case 'top':
      const memUsed = osState.processes.reduce((a, p) => a + p.mem, 0);
      output = `Tasks: ${osState.processes.length} total, 1 running\n`;
      output += `Mem:   ${memUsed}MB used, 4096MB free\n\n`;
      output += osState.processes
        .map(p => `${p.pid}  ${p.user}  ${p.name}  ${p.cpu}%  ${p.mem}MB`)
        .join('\n');
      break;
      
    case 'ifconfig':
    case 'ip':
      output = osState.network.interfaces
        .map(i => `${i.name}: flags=${i.status === 'up' ? 'UP' : 'DOWN'} mtu 1500\n        inet ${i.ip}  netmask 255.255.255.0`)
        .join('\n\n');
      break;
      
    case 'df':
      output = 'Filesystem     1K-blocks    Used Available Use% Mounted on\n';
      output += '/dev/root        1638400   524288   1114112  32% /\n';
      output += '/dev/sda1        2048000   102400   1945600   5% /home\n';
      break;
      
    case 'free':
      output = '              total        used        free      shared  buff/cache   available\n';
      output += 'Mem:        4194304     1048576     3145728      51200     524288     2097152\n';
      output += 'Swap:       1048576           0     1048576\n';
      break;
      
    case 'uptime':
      const uptime = Math.floor((Date.now() - osState.uptime) / 1000);
      const hours = Math.floor(uptime / 3600);
      const mins = Math.floor((uptime % 3600) / 60);
      output = ` ${new Date().toLocaleTimeString()} up ${hours}:${mins.toString().padStart(2, '0')}, 1 user, load average: 0.${Math.floor(Math.random()*50)}, 0.${Math.floor(Math.random()*30)}, 0.${Math.floor(Math.random()*20)}`;
      break;
      
    case 'clear':
      output = '__CLEAR__';
      break;
      
    case 'help':
      output = `Adizilla OS v1.0.0 - Available Commands:
  
  File Commands:    ls, cd, pwd, cat, mkdir, touch, rm, cp, mv
  Process Commands: ps, top, kill
  Network Commands: ping, ifconfig, netstat
  System Commands:  uname, date, uptime, free, df
  User Commands:    whoami, login, logout, passwd
  Other:            clear, echo, help, history
  
Type 'help <command>' for more info.`;
      break;
      
    case 'history':
      output = osState.commandHistory.map((c, i) => `  ${i + 1}  ${c}`).join('\n');
      break;
      
    case 'exit':
      output = 'logout';
      break;
      
    default:
      // Check aliases
      if (osState.aliases[cmd]) {
        const aliasedCmd = osState.aliases[cmd];
        // Recursively handle alias (simple version)
        output = `Alias: ${cmd} -> ${aliasedCmd}`;
      } else {
        output = `bash: ${cmd}: command not found`;
        exitCode = 127;
      }
  }
  
  res.json({ output, exitCode, cwd });
});

// Catch-all for SPA
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, '../frontend/index.html'));
});

// Start server
app.listen(PORT, () => {
  console.log(`
╔═══════════════════════════════════════════════════════════╗
║                                                           ║
║     █████╗ ██████╗ ██████╗███████╗███████╗███████╗        ║
║    ██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝        ║
║    ███████║██████╔╝██║     █████╗  ███████╗███████╗        ║
║    ██╔══██║██╔══██╗██║     ██╔══╝  ╚════██║╚════██║        ║
║    ██║  ██║██║  ██║╚██████╗███████╗███████║███████║        ║
║    ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚══════╝╚══════╝╚══════╝        ║
║                      Mobile OS                             ║
║                                                           ║
║         🇰🇪 Built with ❤️ in Nairobi, Kenya              ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝

  Server running on http://localhost:${PORT}
  Type 'help' for available commands
`);
  log('system_start', { version: '1.0.0' });
});
