"use strict";
let fs = require("fs")
let s = fs.readFileSync(process.argv[2], "utf8")
let m = /(0x[a-f0-9]+)\s*logStoreUF2\s*$/m.exec(s)
console.log(m[1])
