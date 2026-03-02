// Adizilla OS - Core System Module
// Handles OS state, process management, file system, and core functionality

const AdzOS = {
  // OS State
  state: {
    initialized: false,
    booted: false,
    loggedIn: false,
    currentUser: null,
    currentPath: '/home/root',
    theme: 'dark',
    uptime: 0,
    startTime: null,
    commandHistory: [],
    historyIndex: -1,
    aliases: {},
    processes: [],
    files: {},
    network: {},
    settings: {
      soundEnabled: true,
      animationsEnabled: true,
      autoUpdate: true
    }
  },
  
  // Initialize the OS
  async init() {
    console.log('🚀 Initializing Adizilla OS...');
    
    // Start boot sequence
    await this.boot();
    
    // Initialize core systems
    await this.initFileSystem();
    await this.loadUserData();
    await this.loadSettings();
    
    this.state.initialized = true;
    console.log('✅ Adizilla OS initialized');
    
    // Show login screen
    this.showLogin();
  },
  
  // Boot sequence with animation
  async boot() {
    const bootScreen = document.getElementById('boot-screen');
    const bootText = document.getElementById('boot-text');
    const bootFill = document.getElementById('boot-fill');
    
    const bootSteps = [
      { text: 'Initializing kernel...', progress: 10 },
      { text: 'Loading memory manager...', progress: 25 },
      { text: 'Starting process scheduler...', progress: 40 },
      { text: 'Mounting virtual file system...', progress: 55 },
      { text: 'Initializing network stack...', progress: 70 },
      { text: 'Loading user interface...', progress: 85 },
      { text: 'Starting system services...', progress: 95 },
      { text: 'Welcome to Adizilla OS!', progress: 100 }
    ];
    
    for (const step of bootSteps) {
      bootText.textContent = step.text;
      bootFill.style.width = step.progress + '%';
      await this.delay(300 + Math.random() * 200);
    }
    
    this.state.startTime = Date.now();
    this.state.booted = true;
    
    // Hide boot screen
    await this.delay(500);
    bootScreen.classList.add('hidden');
  },
  
  // Show login screen
  showLogin() {
    const loginScreen = document.getElementById('login-screen');
    loginScreen.classList.remove('hidden');
  },
  
  // Handle login
  async login(username, password) {
    try {
      const result = await API.login(username, password);
      
      if (result.success) {
        this.state.loggedIn = true;
        this.state.currentUser = result.user;
        this.state.currentPath = '/home/' + result.user.username;
        
        // Hide login, show desktop
        document.getElementById('login-screen').classList.add('hidden');
        document.getElementById('desktop').classList.remove('hidden');
        
        // Start clock
        this.startClock();
        
        // Load terminal
        this.openApp('terminal');
        
        console.log(`👤 User logged in: ${username}`);
        return true;
      }
    } catch (error) {
      console.error('Login failed:', error);
      return false;
    }
  },
  
  // Handle logout
  async logout() {
    await API.logout();
    
    this.state.loggedIn = false;
    this.state.currentUser = null;
    
    document.getElementById('desktop').classList.add('hidden');
    this.showLogin();
  },
  
  // Initialize virtual file system
  async initFileSystem() {
    // Default file structure
    this.state.files = {
      '/': { type: 'dir', perms: 'rwxr-xr-x' },
      '/home': { type: 'dir', perms: 'rwxr-xr-x' },
      '/home/root': { type: 'dir', perms: 'rwxr-xr-x' },
      '/home/root/files': { type: 'dir', perms: 'rwxr-xr-x' },
      '/home/root/welcome.txt': { 
        type: 'file', 
        perms: 'rw-r--r--',
        content: 'Welcome to Adizilla OS!\n\nBuilt with ❤️ in Nairobi, Kenya 🇰🇪\n\nType "help" for available commands.'
      },
      '/etc': { type: 'dir', perms: 'rwxr-xr-x' },
      '/etc/hostname': { type: 'file', perms: 'rw-r--r--', content: 'adizilla' },
      '/etc/os-release': { 
        type: 'file', 
        perms: 'r--r--r--', 
        content: 'NAME="Adizilla Mobile OS"\nVERSION="1.0.0"\nBUILT_BY="Lila, Nairobi"' 
      },
      '/var': { type: 'dir', perms: 'rwxr-xr-x' },
      '/tmp': { type: 'dir', perms: 'rwxrwxrwx' },
      '/bin': { type: 'dir', perms: 'rwxr-xr-x' },
      '/usr': { type: 'dir', perms: 'rwxr-xr-x' }
    };
  },
  
  // Load user data
  async loadUserData() {
    try {
      // Load command history
      const history = await API.getHistory();
      this.state.commandHistory = history;
      
      // Load aliases
      const aliases = await API.getAliases();
      this.state.aliases = aliases;
      
      // Load processes
      const processes = await API.getProcesses();
      this.state.processes = processes;
      
      // Load network info
      const network = await API.getNetwork();
      this.state.network = network;
    } catch (error) {
      console.warn('Could not load user data:', error);
    }
  },
  
  // Load settings
  async loadSettings() {
    try {
      const system = await API.system();
      this.state.uptime = system.uptime;
      
      // Load theme preference
      const savedTheme = localStorage.getItem('adz-theme');
      if (savedTheme) {
        this.setTheme(savedTheme);
      }
    } catch (error) {
      console.warn('Could not load settings:', error);
    }
  },
  
  // File system operations
  async listFiles(path) {
    try {
      return await API.listFiles(path);
    } catch (error) {
      console.error('Error listing files:', error);
      return {};
    }
  },
  
  async readFile(path) {
    try {
      return await API.readFile(path);
    } catch (error) {
      return { error: error.message };
    }
  },
  
  async writeFile(path, content) {
    try {
      return await API.writeFile(path, content);
    } catch (error) {
      return { error: error.message };
    }
  },
  
  async createDir(path) {
    try {
      return await API.mkdir(path);
    } catch (error) {
      return { error: error.message };
    }
  },
  
  async deleteFile(path) {
    try {
      return await API.deleteFile(path);
    } catch (error) {
      return { error: error.message };
    }
  },
  
  // Process management
  async getProcesses() {
    try {
      return await API.getProcesses();
    } catch (error) {
      return [];
    }
  },
  
  async createProcess(name, command) {
    try {
      return await API.createProcess(name, command);
    } catch (error) {
      return { error: error.message };
    }
  },
  
  async killProcess(pid) {
    try {
      return await API.killProcess(pid);
    } catch (error) {
      return { error: error.message };
    }
  },
  
  // Execute terminal command
  async executeCommand(command) {
    try {
      // Save to history
      if (command.trim()) {
        this.state.commandHistory.push(command);
        await API.addToHistory(command);
      }
      
      // Handle aliases
      const cmd = command.trim().split(/\s+/)[0];
      if (this.state.aliases[cmd]) {
        command = command.replace(cmd, this.state.aliases[cmd]);
      }
      
      return await API.executeCommand(command, this.state.currentPath);
    } catch (error) {
      return { output: `Error: ${error.message}`, exitCode: 1 };
    }
  },
  
  // Navigation
  changePath(newPath) {
    if (newPath === '..') {
      const parts = this.state.currentPath.split('/').filter(Boolean);
      parts.pop();
      this.state.currentPath = '/' + parts.join('/') || '/';
    } else if (newPath.startsWith('/')) {
      this.state.currentPath = newPath;
    } else {
      this.state.currentPath = this.state.currentPath === '/' 
        ? '/' + newPath 
        : this.state.currentPath + '/' + newPath;
    }
  },
  
  // Theme
  setTheme(theme) {
    this.state.theme = theme;
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('adz-theme', theme);
  },
  
  // Clock
  startClock() {
    const updateClock = () => {
      const now = new Date();
      const hours = now.getHours().toString().padStart(2, '0');
      const minutes = now.getMinutes().toString().padStart(2, '0');
      const clock = document.getElementById('clock');
      if (clock) {
        clock.textContent = `${hours}:${minutes}`;
      }
    };
    
    updateClock();
    setInterval(updateClock, 1000);
  },
  
  // Get uptime
  getUptime() {
    if (!this.state.startTime) return '0:00:00';
    
    const elapsed = Math.floor((Date.now() - this.state.startTime) / 1000);
    const hours = Math.floor(elapsed / 3600);
    const minutes = Math.floor((elapsed % 3600) / 60);
    const seconds = elapsed % 60;
    
    return `${hours}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
  },
  
  // Utility
  delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  },
  
  // Notifications
  notify(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    document.body.appendChild(notification);
    
    setTimeout(() => {
      notification.remove();
    }, 3000);
  }
};

// Export for use
window.AdzOS = AdzOS;
