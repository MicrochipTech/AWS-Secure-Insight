/*
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

const electron = require('electron');
const app = electron.app;  // Module to control application life.
const BrowserWindow = electron.BrowserWindow;  // Module to create native browser window.
const Menu = electron.Menu;
const shell = electron.shell;
//const remote = require('electron').remote;
//var Menu = require('menu');
//const MenuItem = remote.MenuItem;

//Create the window Menu for OSX.
if(process.platform == 'darwin'){
var template = [
  {
    label: 'Microchip Insight on Things',
    submenu: [
      {
        label: 'About Insight on Things',
        // selector: 'orderFrontStandardAboutPanel:'
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/about.html'); }
      },
      {
        type: 'separator'
      },
      {
        label: 'Services',
        submenu: []
      },
      {
        type: 'separator'
      },
      {
        label: 'Hide IoT',
        accelerator: 'Command+H',
        selector: 'hide:'
      },
      {
        label: 'Hide Others',
        accelerator: 'Command+Shift+H',
        selector: 'hideOtherApplications:'
      },
      {
        label: 'Show All',
        selector: 'unhideAllApplications:'
      },
      {
        type: 'separator'
      },
      {
        label: 'Quit',
        accelerator: 'Command+Q',
        click: function() { app.quit(); }
      },
    ]
  },
  {
    label: 'Edit',
    submenu: [
      {
        label: 'Cut',
        accelerator: 'Command+X',
        selector: 'cut:',
      },
      {
        label: 'Copy',
        accelerator: 'Command+C',
        selector: 'copy:',
      },
      {
        label: 'Paste',
        accelerator: 'Command+V',
        selector: 'paste:',
      },
    ]
  },
  {
    label: 'View',
    submenu: [
      {
        label: 'Thing Shadow',
        accelerator: 'Command+T',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/active.html'); }
      },
      {
        label: 'Reload',
        accelerator: 'Command+R',
        click: function() { BrowserWindow.getFocusedWindow().reload(); }
      },
      {
        label: 'Toggle DevTools',
        accelerator: 'Alt+Command+I',
        click: function() { BrowserWindow.getFocusedWindow().toggleDevTools(); }
      },
    ]
  },
  {
    label: 'Thing',
    submenu: [
      {
        label: 'AWS Settings',
        accelerator: 'Shift+Command+A',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/settings.html'); }
      },
      {
        label: 'Security Settings',
        accelerator: 'Shift+Command+S',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/securething.html'); }
      },
    ]
  },
  {
    label: 'Window',
    submenu: [
      {
        label: 'Minimize',
        accelerator: 'Command+M',
        selector: 'performMiniaturize:'
      },
      {
        label: 'Close',
        accelerator: 'Command+W',
        selector: 'performClose:'
      },
      {
        type: 'separator'
      },
      {
        label: 'Bring All to Front',
        selector: 'arrangeInFront:'
      },
    ]
  },
  {
    label: 'Help',
    submenu: [
      {
        label: 'Secure Insight on Things User Guide',
        click: function() {
          shell.openExternal('https://github.com/MicrochipTech/AWS-Secure-Insight/wiki/User%20Guide');}
      },
      {
        label: 'Microchip IoT Landing page',
        click: function() {
          shell.openExternal('http://www.microchip.com/iot');}
      },
      {
        label: 'Microchip Home Page',
        click: function() {
          shell.openExternal('http://www.microchip.com');}
      },
    ]
  },
];};

//Create the window Menu for Windows and Linux.
if(process.platform != 'darwin'){
var template = [
  {
    label: 'Microchip Insight on Things',
    submenu: [
      {
        label: 'About Insight on Things',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/about.html'); }
      },
      {
        label: 'Quit',
        accelerator: 'Command+Q',
        click: function() { app.quit(); }
      },
    ]
  },
  {
    label: 'View',
    submenu: [
      {
        label: 'Thing Shadow',
        accelerator: 'Command+T',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/active.html'); }
      },
      {
        label: 'Reload',
        accelerator: 'Command+R',
        click: function() { BrowserWindow.getFocusedWindow().reloadIgnoringCache(); }
      },
      {
        label: 'Toggle DevTools',
        accelerator: 'Alt+Command+I',
        click: function() { BrowserWindow.getFocusedWindow().toggleDevTools(); }
      },
    ]
  },
  {
    label: 'Thing',
    submenu: [
      {
        label: 'AWS Settings',
        accelerator: 'Shift+Command+A',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/settings.html'); }
      },
       {
        label: 'Security Settings',
        accelerator: 'Shift+Command+S',
        click: function() { mainWindow.loadURL('file://' + __dirname + '/content/securething.html'); }
      },
   ]
  },
  {
    label: 'Help',
    submenu: [
      {
        label: 'Secure Insight on Things User Guide',
        click: function() {
          shell.openExternal('https://github.com/MicrochipTech/AWS-Secure-Insight/wiki/User%20Guide');}
      },
      {
        label: 'Microchip IoT Landing page',
        click: function() {
          shell.openExternal('http://www.microchip.com/iot');}
      },
      {
        label: 'Microchip Home Page',
        click: function() {
          shell.openExternal('http://www.microchip.com');}
      },
    ]
  },
];};




// Report crashes to our server.
//electron.crashReporter.start();

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;

// Quit when all windows are closed.
app.on('window-all-closed', function() {
  // On OS X it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  /*if (process.platform != 'darwin') {
    app.quit();
  }*/
  app.quit();
});


// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
app.on('ready', function() {
  // Create the browser window.
  mainWindow = new BrowserWindow({width: 600 , height: 630, resizable: true});

  // and load the index.html of the app.
  // mainWindow.loadURL('file://' + __dirname + '/index.html');
  mainWindow.loadURL('file://' + __dirname + '/content/securething.html');

  // Open the DevTools.
  //mainWindow.webContents.openDevTools();

  // Emitted when the window is closed.
  mainWindow.on('closed', function() {
    // Dereference the window object, usually you would store windows
    // in an array if your app supports multi windows, this is the time
    // when you should delete the corresponding element.
    mainWindow = null;

  });
  //Now we can generate the system Menus

  var menu = Menu.buildFromTemplate(template);

  Menu.setApplicationMenu(menu);
});
