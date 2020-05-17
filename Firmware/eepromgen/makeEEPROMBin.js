const fs = require("fs");
const PNG = require("pngjs").PNG;

let project;

if (!process.argv[2] || !process.argv[3]) {
    console.log("Usage:\nnode makeHeader.js <file> fullscreen|font <image name>|<char width>");
    process.exit();
}

try {
    project = JSON.parse(fs.readFileSync(process.argv[2]).toString());
} catch (e) {
    console.log(e);
    console.log("Invalid project file!");
    console.log("Usage:\nnode makeHeader.js <file> fullscreen|font <image name>|<char width>");
    process.exit();
}

let binaries = [];

for (let projEl of project) {
    console.log(projEl.file);
    let image = PNG.sync.read(fs.readFileSync(__dirname + "\\" + projEl.file));
    switch (projEl.type) {
        case "fullscreen":
            binaries.push(generateFullscreenBin(image));
            break;
        case "font":
            binaries.push(generateFontBin(image, parseInt(projEl.charWidth)));
            break;
    }
}

fs.writeFileSync(process.argv[3], Buffer.concat(binaries));

function generateFullscreenBin(image) {
    let binary = Buffer.alloc(image.width * image.height / 8);
    let pos = 0;
    
    for (let y = 0; y < 64; y++) {
        for (let x = 0; x < 128; x += 8) {
            let pixelByte = 0;
            for (let i = 0; i < 8; i++) {
                let color = image.data[y * 128 * 4 + (x + i) * 4] == 0x00;
                pixelByte += color << (7 - i);
            }
            binary[pos] = pixelByte;
            pos++;
        }
    }
    
    return binary;
}

function generateFontBin(image, charWidth) {
    let binary = Buffer.alloc(image.height * (image.width / charWidth));
    let pos = 0;

    for (let character = 0; character < image.width; character += charWidth) {
        for (let y = 0; y < image.height; y++) {
            let currByte = 0;
            for (let x = 0; x < charWidth; x++) {
                let color = image.data[y * image.width * 4 + (character + x) * 4 + 3] == 0xFF;
                currByte += color << (charWidth - x - 1);
            }
            binary[pos] = currByte;
            pos++;
        }
    }
    return binary;
}