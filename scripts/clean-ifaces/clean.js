let fs = require("fs")
let plist = require("plist")
let bkp = fs.readFileSync("/Library/Preferences/SystemConfiguration/preferences.plist", "utf8")
let obj = plist.parse(bkp)

let rem = {}
for (let k of Object.keys(obj.NetworkServices)) {
  let o = obj.NetworkServices[k]
  if (/usbmodem/.test(o.Interface.DeviceName)) {
    console.log("REMOVE", k, o.UserDefinedName)
    rem[k] = 1
  }
}

rec(obj)


let fn = "preferences.plist"
fs.writeFileSync(fn, plist.build(obj))
console.log("wrote", fn)
fs.writeFileSync("old-" + fn, bkp)

function rec(o) {
  if (Array.isArray(o)) {
    for (let i = 0; i < o.length; ++i) {
      while (rem.hasOwnProperty(o[i]))
        o.splice(i, 1)
    }
    o.forEach(rec)
  } else if (typeof o == "object") {
    for (let k of Object.keys(o)) {
      if (rem.hasOwnProperty(k)) {
        delete o[k]
      } else {
        rec(o[k])
      }
    }
  }
}
