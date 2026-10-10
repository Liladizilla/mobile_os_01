// Dedus OS - UX prototype UI module
// Handles window management, drag, resize, and user interface

const UI = {
  windows: [],
  activeWindow: null,
  zIndex: 100,
  
  // Initialize UI
  init() {
    this.setupEventListeners();
    this.setupKeyboardShortcuts();
  },
  
  // Setup event listeners
  setupEventListeners() {
    // Desktop icon clicks
    document.querySelectorAll('.desktop-icon').forEach(icon => {
      icon.addEventListener('click', () => {
        const app = icon.dataset.app;
        if (app) this.openApp(app);
      });
    });
    
    // Taskbar app clicks
    document.querySelectorAll('.taskbar-item').forEach(item => {
      item.addEventListener('click', () => {
        const app = item.dataset.app;
        if (app) this.openApp(app);
      });
    });
    
    // Window controls
    document.addEventListener('click', (e) => {
      if (e.target.classList.contains('minimize')) {
        this.minimizeWindow(e.target.closest('.window'));
      } else if (e.target.classList.contains('maximize')) {
        this.maximizeWindow(e.target.closest('.window'));
      } else if (e.target.classList.contains('close')) {
        this.closeWindow(e.target.closest('.window'));
      }
    });
    
    // Focus window on header click
    document.addEventListener('mousedown', (e) => {
      const header = e.target.closest('.window-header');
      if (header) {
        this.focusWindow(header.closest('.window'));
      }
    });
    
    // Right-click context menu
    document.addEventListener('contextmenu', (e) => {
      e.preventDefault();
    });
  },
  
  // Setup keyboard shortcuts
  setupKeyboardShortcuts() {
    document.addEventListener('keydown', (e) => {
      // Ctrl+Alt+T - Open terminal
      if (e.ctrlKey && e.altKey && e.key === 'T') {
        e.preventDefault();
        this.openApp('terminal');
      }
      
      // Alt+F4 - Close window
      if (e.altKey && e.key === 'F4') {
        if (this.activeWindow) {
          e.preventDefault();
          this.closeWindow(this.activeWindow);
        }
      }
    });
  },
  
  // Open an application
  openApp(appName) {
    // Check if window already exists
    let window = document.querySelector(`.window[data-app="${appName}"]`);
    
    if (window) {
      // Restore if minimized
      if (window.classList.contains('minimized')) {
        window.classList.remove('minimized');
      }
      this.focusWindow(window);
      return;
    }
    
    // Create new window from template
    const template = document.getElementById(`${appName}-template`);
    if (!template) {
      console.warn(`No template found for app: ${appName}`);
      return;
    }
    
    const clone = template.content.cloneNode(true);
    const windowEl = clone.querySelector('.window');
    
    document.getElementById('windows').appendChild(windowEl);
    
    // Setup window
    this.setupWindow(windowEl);
    this.focusWindow(windowEl);
    
    // Initialize app
    if (appName === 'terminal') {
      Terminal.init(windowEl);
    } else if (appName === 'files') {
      FileManager.init(windowEl);
    } else if (appName === 'settings') {
      Settings.init(windowEl);
    }
  },
  
  // Setup window behavior
  setupWindow(window) {
    // Make draggable
    const header = window.querySelector('.window-header');
    let isDragging = false;
    let startX, startY, initialX, initialY;
    
    header.addEventListener('mousedown', (e) => {
      if (e.target.classList.contains('window-btn')) return;
      
      isDragging = true;
      startX = e.clientX;
      startY = e.clientY;
      initialX = window.offsetLeft;
      initialY = window.offsetTop;
      window.style.transition = 'none';
    });
    
    document.addEventListener('mousemove', (e) => {
      if (!isDragging) return;
      
      const dx = e.clientX - startX;
      const dy = e.clientY - startY;
      
      window.style.left = (initialX + dx) + 'px';
      window.style.top = (initialY + dy) + 'px';
    });
    
    document.addEventListener('mouseup', () => {
      isDragging = false;
      window.style.transition = '';
    });
    
    // Add to windows array
    this.windows.push(window);
  },
  
  // Focus a window
  focusWindow(window) {
    if (this.activeWindow) {
      this.activeWindow.classList.remove('active');
    }
    
    window.classList.add('active');
    this.activeWindow = window;
    this.zIndex++;
    window.style.zIndex = this.zIndex;
    
    // Update taskbar
    const appName = window.dataset.app;
    document.querySelectorAll('.taskbar-item').forEach(item => {
      item.classList.toggle('active', item.dataset.app === appName);
    });
  },
  
  // Minimize window
  minimizeWindow(window) {
    window.classList.add('minimized');
    window.classList.remove('active');
    
    if (this.activeWindow === window) {
      this.activeWindow = null;
    }
  },
  
  // Maximize window
  maximizeWindow(window) {
    window.classList.toggle('maximized');
    this.focusWindow(window);
  },
  
  // Close window
  closeWindow(window) {
    window.classList.add('closing');
    
    setTimeout(() => {
      window.remove();
      this.windows = this.windows.filter(w => w !== window);
      
      if (this.activeWindow === window) {
        this.activeWindow = null;
      }
    }, 150);
  }
};

// Export
window.UI = UI;
