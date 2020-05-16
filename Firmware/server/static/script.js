const socket = io();

let canvas = document.querySelector("canvas");
let ctx = canvas.getContext("2d");

socket.on("screen", data => {
    let scrBuf = new Uint8Array(data);
    let index = 0;
    for (let page = 0; page < 8; page++) {
        for (let x = 0; x < 128; x++) {
            for (let y = 0; y < 8; y++) {
                ctx.fillStyle = ((scrBuf[index] >> y) & 1) ? "#000000" : "#FFFFFF";
                ctx.fillRect(x, page * 8 + y, 1, 1);
            }
            index++;
        }
    }
});

socket.on("crash", data => {
    console.log(data);
    let canvas = document.querySelector("canvas");
    let ctx = canvas.getContext("2d");
    ctx.fillStyle = "#000000";
    ctx.fillText("You crashed me :(", 20, 32);
});

const charTable = {
    48: 0,
    49: 1,
    50: 2,
    51: 3,
    52: 4,
    53: 5,
    54: 6,
    55: 7,
    56: 8,
    57: 9,
};

document.addEventListener("keypress", ev => {
    socket.emit("button", charTable[ev.charCode]);
});