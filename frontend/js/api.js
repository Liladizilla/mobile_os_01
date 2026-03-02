// Adizilla OS - API Module
// Handles all communication with the backend server

const API = {
  baseUrl: '',
  sessionId: null,
  currentUser: null,
  
  // Set base URL
  setBaseUrl(url) {
    this.baseUrl = url;
  },
  
  // Generic request handler
  async request(endpoint, options = {}) {
    const url = this.baseUrl + endpoint;
    const config = {
      headers: {
        'Content-Type': 'application/json',
        ...(this.sessionId && { 'Authorization': `Bearer ${this.sessionId}` })
      },
      ...options
    };
    
    if (config.body && typeof config.body === 'object') {
      config.body = JSON.stringify(config.body);
    }
    
    try {
      const response = await fetch(url, config);
      const data = await response.json();
      
      if (!response.ok) {
        throw new Error(data.error || 'Request failed');
      }
      
      return data;
    } catch (error) {
      console.error('API Error:', error);
      throw error;
    }
  },
  
  // Health check
  async health() {
    return this.request('/api/health');
  },
  
  // System info
  async system() {
    return this.request('/api/system');
  },
  
  // Process management
  async getProcesses() {
    return this.request('/api/processes');
  },
  
  async createProcess(name, command) {
    return this.request('/api/processes', {
      method: 'POST',
      body: { name, command }
    });
  },
  
  async killProcess(pid) {
    return this.request(`/api/processes/${pid}`, {
      method: 'DELETE'
    });
  },
  
  // File system
  async listFiles(path = '/') {
    return this.request(`/api/fs?path=${encodeURIComponent(path)}`);
  },
  
  async readFile(path) {
    return this.request(`/api/fs/read?path=${encodeURIComponent(path)}`);
  },
  
  async writeFile(path, content) {
    return this.request('/api/fs/write', {
      method: 'POST',
      body: { path, content }
    });
  },
  
  async deleteFile(path) {
    return this.request('/api/fs/delete', {
      method: 'POST',
      body: { path }
    });
  },
  
  async mkdir(path) {
    return this.request('/api/fs/mkdir', {
      method: 'POST',
      body: { path }
    });
  },
  
  // User authentication
  async login(username, password) {
    const data = await this.request('/api/auth/login', {
      method: 'POST',
      body: { username, password }
    });
    
    if (data.sessionId) {
      this.sessionId = data.sessionId;
      this.currentUser = data.user;
    }
    
    return data;
  },
  
  async logout() {
    const data = await this.request('/api/auth/logout', {
      method: 'POST'
    });
    
    this.sessionId = null;
    this.currentUser = null;
    
    return data;
  },
  
  async getCurrentUser() {
    try {
      return await this.request('/api/auth/me');
    } catch {
      return null;
    }
  },
  
  async getUsers() {
    return this.request('/api/users');
  },
  
  // Network
  async getNetwork() {
    return this.request('/api/network');
  },
  
  async ping(host) {
    return this.request('/api/network/ping', {
      method: 'POST',
      body: { host }
    });
  },
  
  // Terminal
  async executeCommand(command, cwd = '/home/root') {
    return this.request('/api/terminal/exec', {
      method: 'POST',
      body: { command, cwd }
    });
  },
  
  // History and aliases
  async getHistory() {
    return this.request('/api/history');
  },
  
  async addToHistory(command) {
    return this.request('/api/history', {
      method: 'POST',
      body: { command }
    });
  },
  
  async getAliases() {
    return this.request('/api/aliases');
  },
  
  async addAlias(alias, command) {
    return this.request('/api/aliases', {
      method: 'POST',
      body: { alias, command }
    });
  },
  
  // Logs
  async getLogs(limit = 50) {
    return this.request(`/api/logs?limit=${limit}`);
  }
};

// Export for use in other modules
window.API = API;
