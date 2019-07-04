const net = require("net");
const port = process.env.PORT ? process.env.PORT - 100 : 3000;

process.env.ELECTRON_START_URL = `http://localhost:${port}`;

const client = new net.Socket();

let startedElectron = false;
const tryConnection = () =>
  client.connect({ port }, () => {
    client.end();
    if (!startedElectron) {
      console.log("Iniciando electron");
      startedElectron = true;
      const exec = require("child_process").exec;
      exec("yarn run electron");
    }
  });

tryConnection();

client.on("error", error => {
  setTimeout(tryConnection, 1000);
});
