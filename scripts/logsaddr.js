"use strict";
let fs = require("fs")
let s = fs.readFileSync("build/uf2-bootloader.map", "utf8")
let m = /logStoreUF2\n\r?.*?(0x[a-f0-9]+)/.exec(s)
console.log(m[1])
