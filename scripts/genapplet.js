let fs = require("fs")
let s = fs.readFileSync(process.argv[2], "utf8")
let pref = ""
let infun = false
let words = []
for (let l of s.split(/\n/)) {
    let m = /^00000000 <(.*)>:/.exec(l)
    if (m && m[1] == process.argv[3]) infun = true
    if (/^Disassembly/.test(l)) infun = false
    if (!infun) continue
    m = /^\s*[0-9a-f]+:\s+([0-9a-f]+)\s+/.exec(l)
    if (m) {
        let n = m[1]
        if (n.length == 4) {
            if (pref) {
                words.push("0x" + n + pref)
                pref = ""
            } else {
                pref = n
            }
        } else if (n.length == 8) {
            if (pref) throw new Error()
            words.push("0x" + n)
        } else {
            throw new Error()
        }
    }
}

let r = ""
for (let i = 0; i < words.length; i++) {
    if (i%6 == 0) r += "\n"
    r += words[i] + ", "
}

console.log(r)