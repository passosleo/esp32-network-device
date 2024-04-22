const net = require("net");

const server = net.createServer((socket) => {
  console.log("Client connected");

  socket.on("data", (data) => {
    const [macAddress, mL] = data.toString().split(";");

    const info = {
      macAddress,
      totalMilliliters: Number(mL),
      totalLiters: Number(mL) / 1000,
    };

    console.log(info);
  });

  socket.on("end", () => {
    console.log("Client disconnected");
  });
});

const PORT = 3000;

server.listen(PORT, () => {
  console.log(`Server started at port ${PORT}`);
});
