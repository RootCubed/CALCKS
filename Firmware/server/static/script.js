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
    13: 4,
    46: 3,
    48: 2,
    49: 6,
    50: 7,
    51: 8,
    52: 11,
    53: 12,
    54: 13,
    55: 16,
    56: 17,
    57: 18,
};

document.addEventListener("keypress", ev => {
    if (charTable[ev.charCode]) {
        socket.emit("button", charTable[ev.charCode]);
    }
});