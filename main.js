const { app, BrowserWindow } = require("electron");
const path = require("path");
const url = require("url");

let win;

/**@function creatrWindow inicializa aplicação com formato de janela. */

const createWindow = () => {
  console.log("Deu tudo certo");
  win = new BrowserWindow({
    width: 800,
    height: 600,
    minWidth: 500,
    webPreferences: {
      nodeIntegration: true
    }
  });

  const startUrl =
    process.env.ELECTRON_START_URL ||
    url.format({
      pathname: path.join(__dirname, "/../build/index.html"),
      protocol: "file:",
      slashes: true
    });

  win.loadURL(startUrl);

  win.on("closed", () => {
    win = null;
  });
};

app.on("ready", createWindow);

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});

app.on("activate", () => {
  if (win === null) {
    createWindow();
  }
});
