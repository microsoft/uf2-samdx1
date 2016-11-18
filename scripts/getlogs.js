"use strict";
let fs = require("fs")
let s = fs.readFileSync("logs.tmp", "utf8")
let bufs = []
for (let line of s.split("\n")) {
  let m = /0x[a-f0-9]+: ([a-f0-9 ]+)/.exec(line)
  if (!m) continue
  bufs.push(new Buffer(m[1].replace(/ /g, ""), "hex"))
}
let total = Buffer.concat(bufs)
let ptr = total.indexOf("LOGHEADER_42_42")
if (ptr < 0) {
  console.log("No logs.")
} else {
  let len = total.readUInt32LE(ptr + 16)
  console.log("*\n* Logs\n*\n")
  console.log(total.slice(ptr + 20, ptr + 20 + len).toString("binary"))
}
