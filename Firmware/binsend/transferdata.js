const SerialPort = require("serialport");
const port = SerialPort("COM3", {
    baudRate: 9600,
    dataBits: 8,
    parity: "none",
    stopBits: 1
});

const fs = require("fs");

let binary = fs.readFileSync(process.argv[2]);

let position = 0;

port.on("data", data => {
    console.log(`> ${data}`);
    if (data == "s") {
        let lenBuf = Buffer.alloc(2);
        lenBuf.writeInt16BE(binary.length);
        port.write(lenBuf);
    }
    if (data == "n") {
        transferBatch();
    }
    if (data == "d") {
        console.log("yea he done");
        setTimeout(process.exit, 1000);
    }
});

function transferBatch() {
    console.log(position + "/" + binary.length);
    if (position + 1024 > binary.length) {
        port.write(binary.subarray(position, binary.length));
        console.log(binary.length + "/" + binary.length);
        return;
    } else {
        port.write(binary.subarray(position, position + 1024));
    }
    position += 1024;
}