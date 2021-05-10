const express = require("express");
const app = express();
const http = require("http").createServer(app);
var io = require("socket.io")(http);
const port = 3000;

const { spawn } = require("child_process");

let runningCalcs = {};

io.on("connection", socket => {
    runningCalcs[socket.id] = [spawn(__dirname + "/calc.exe", {encoding: "binary"}), Buffer.alloc(128 * 8), undefined];
    let userCalc = runningCalcs[socket.id][0];
    userCalc.stdout.on("data", data => {
        try {
            socket.emit("screen", data);
        } catch(e) {}
    });
    
    userCalc.stderr.on("data", data => {
        console.log(`stderr: ${data}`);
        try {
            socket.emit("error", data);
        } catch(e) {}
    });
    
    userCalc.on("error", (error) => {
        console.log(`error: ${error.message}`);
    });

    userCalc.on("close", function (data) {
        try {
            socket.emit("crash", data);
            clearInterval(runningCalcs[socket.id][2]);
            userCalc.kill();
            runningCalcs[socket.id] = undefined;
        } catch(e) {}
    })

    runningCalcs[socket.id][2] = setInterval(() => {
        userCalc.stdin.write('g');
    }, 200);

    socket.on("button", btn => {
        if (btn != undefined) {
            userCalc.stdin.write('b');
            userCalc.stdin.write(btn.toString());
        }
    });

    socket.on("specialbutton", btn => {
        if (btn != undefined) {
            userCalc.stdin.write('s');
            userCalc.stdin.write(btn.toString());
        }
    });

    socket.on("disconnect", () => {
        clearInterval(runningCalcs[socket.id][2]);
        userCalc.stdin.pause();
        userCalc.kill();
    });
});

app.use(function(req, res, next) {
    res.setHeader("Access-Control-Allow-Origin", "http://calcks.parginli.ch");
    next();
  });

app.use(express.static("static"));

http.listen(port, () => console.log(`Server listening at http://localhost:${port}`));