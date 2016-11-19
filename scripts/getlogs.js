"use strict";
let fs = require("fs")
let s = fs.readFileSync("logs.tmp", "utf8")
let bufs = []
for (let line of s.split("\n")) {
  let m = /0x[a-f0-9]+: ([a-f0-9 ]+)/.exec(line)
  if (!m) continue
  let bb = new Buffer(m[1].replace(/ /g, ""), "hex")
  bufs.push(bb)
}
let total = Buffer.concat(bufs)

let len = total.readUInt32LE(0)
if (len == 0 || len > total.length) {
  console.log("No logs.")
} else {
  console.log("*\n* Logs\n*\n")
  console.log(total.slice(4, 4 + len).toString("binary"))
}