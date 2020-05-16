const express = require("express");
const app = express();
const http = require("http").createServer(app);
var io = require("socket.io")(http);
const port = 3000;

const { spawn } = require("child_process");

let runningCalcs = {};

io.on("connection", socket => {
    runningCalcs[socket.id] = [spawn("calc.exe", {encoding: "binary"}), Buffer.alloc(128 * 8), undefined];
    let userCalc = runningCalcs[socket.id][0];
    let userCalcBuf = runningCalcs[socket.id][1];
    userCalc.stdout.on("data", data => {
        userCalcBuf = Buffer.from(data);
        socket.emit("screen", userCalcBuf);
    });
    
    userCalc.stderr.on("data", data => {
        console.log(`stderr: ${data}`);
        socket.emit("error", data);
    });
    
    userCalc.on("error", (error) => {
        console.log(`error: ${error.message}`);
    });

    userCalc.on("close", function (data) {
        socket.emit("crash", data);
        clearInterval(runningCalcs[socket.id][2]);
        userCalc.kill();
        runningCalcs[socket.id] = undefined;
    })

    runningCalcs[socket.id][2] = setInterval(() => {
        userCalc.stdin.write('g');
    }, 3000);

    socket.on("button", socket => {
        if (socket != undefined) {
            console.log("button", socket, "was pressed!");
            userCalc.stdin.write('b');
            userCalc.stdin.write(socket.toString());
            setTimeout(() => userCalc.stdin.write('g'), 50);
        }
    });
});

io.on("disconnect", socket => {
    let userCalc = runningCalcs[socket.id][0];
    userCalc.stdin.pause();
    userCalc.kill();
    runningCalcs[socket.id] = undefined;
});

app.use(express.static("static"));

http.listen(port, () => console.log(`Server listening at http://localhost:${port}`));