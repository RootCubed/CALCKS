let canvas = document.querySelector("canvas");
let ctx = canvas.getContext("2d");

let getScreen, screenTick, pressButton;
function waitForWasmInit() {
    if (!runtimeInitialized) {
        setTimeout(waitForWasmInit, 10);
        return;
    };

    let screen = Module._malloc(128*8);
    let tmpWrap = Module.cwrap("get_screen", "void", ["number"]);
    
    getScreen = () => {
        tmpWrap(screen);
        return Module.HEAPU8.subarray(screen, screen + 128*8);
    }
    screenTick = Module.cwrap("screen_tick", "void", []);

    pressButton = Module.cwrap("press_button", "void", ["number"]);

    setInterval(dispScreen, 50);
}

setTimeout(waitForWasmInit, 10);

let firstDisplay = true;
function dispScreen() {
    let scrBuf = new Uint8Array(getScreen());
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

    if (firstDisplay) {
        setTimeout(() => firstDisplay = false, 2000);
        return;
    }
    
    for (let i = 0; i < 10; i++) {
        screenTick();
    }
}

const charTable = {
    "Enter": 4,
    ".": 3,
    0: 2,
    1: 6,
    2: 7,
    3: 8,
    4: 11,
    5: 12,
    6: 13,
    7: 16,
    8: 17,
    9: 18,
};

document.addEventListener("keypress", ev => {
    if (charTable[ev.key]) {
        pressButton(charTable[ev.key], false);
    }
});