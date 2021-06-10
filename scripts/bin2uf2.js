#!/usr/bin/env node
"use strict";

let fs = require("fs")
let buf = fs.readFileSync(process.argv[2])

const APP_START_ADDRESS = 0x00002000

const UF2_MAGIC_START0 = 0x0A324655 // "UF2\n"
const UF2_MAGIC_START1 = 0x9E5D5157 // Randomly selected
const UF2_MAGIC_END = 0x0AB16F30   // Ditto

let numBlocks = (buf.length + 255) >> 8
let outp = []
for (let pos = 0; pos < buf.length; pos += 256) {
    let bl = Buffer.alloc(512)
    for (let i = 0; i < 512; ++i)
        bl[i] = 0 // just in case
    bl.writeUInt32LE(UF2_MAGIC_START0, 0)
    bl.writeUInt32LE(UF2_MAGIC_START1, 4)
    bl.writeUInt32LE(0, 8) // flags
    bl.writeUInt32LE(APP_START_ADDRESS + pos, 12)
    bl.writeUInt32LE(256, 16)
    bl.writeUInt32LE(outp.length, 20)
    bl.writeUInt32LE(numBlocks, 24)
    bl.writeUInt32LE(0, 28) // reserved
    for (let i = 0; i < 256; ++i)
        bl[i + 32] = buf[pos + i]
    bl.writeUInt32LE(UF2_MAGIC_END, 512 - 4)
    outp.push(bl)
}

if (numBlocks != outp.length) throw "oops";

let outn = process.argv[3] || "flash.uf2"
fs.writeFileSync(outn, Buffer.concat(outp))
console.log(`Wrote ${numBlocks} blocks to ${outn}`)
