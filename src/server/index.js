const net = require("net");

const TIMEOUT_INTERVAL = 2000;
const clientTimeouts = {};
const clientData = {};

const server = net.createServer((socket) => {
  socket.on("data", (data) => {
    const receivedData = data.toString().trim();

    if (receivedData.length < 1 || !receivedData.includes(";")) return;

    const [macAddress, mL] = receivedData.split(";");

    clearTimeout(clientTimeouts[macAddress]);
    clientData[macAddress] = {
      macAddress,
      totalMilliliters: Number(mL),
      totalLiters: Number(mL) / 1000,
    };

    clientTimeouts[macAddress] = setTimeout(() => {
      console.log(
        `Idle for 2 seconds. Triggering callback for client ${macAddress} with last data received:`
      );
      console.log(clientData[macAddress]);

      clearTimeout(clientTimeouts[macAddress]);
      delete clientTimeouts[macAddress];
      delete clientData[macAddress];
    }, TIMEOUT_INTERVAL);
  });

  socket.on("end", () => {});
});

const PORT = 3000;

server.listen(PORT, () => {
  console.log(`Server started at port ${PORT}`);
});
