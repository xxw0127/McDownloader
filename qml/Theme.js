.pragma library

// ---------------------------------------------------------------------------
// Design tokens for McDownloader
// Ground is a cool grey so white surfaces read as raised; the shell is a deep
// navy that belongs to the same hue family as the brand blue.
// ---------------------------------------------------------------------------

// Surfaces
var ground = "#EDEFF3"
var surface = "#FFFFFF"
var surfaceSunken = "#F3F5F8"
var surfaceHover = "#F4F6FA"
var shell = "#1B2333"
var shellRule = "#2E3849"

// Ink
var ink = "#171A21"
var ink2 = "#444C5C"
var ink3 = "#5C6476"
var ink4 = "#6E7688"
var onShell = "#F3F5F9"
var onShellDim = "#9AA5BA"

// Lines
var line = "#E2E6ED"
var lineSoft = "#EDF0F4"

// Brand
var brand = "#2A6AD9"
var brandHover = "#3A79E8"
var brandPress = "#1F55B4"
var brandSoft = "#E9F1FE"
var brandSoftHover = "#DDE9FD"
var brandInk = "#16407C"

// State
var green = "#3D7A28"
var greenSoft = "#E9F3E2"
var amber = "#9C5F0C"
var amberSoft = "#FBF0DC"
var danger = "#AE3129"
var dangerSoft = "#FBEAE8"

// Geometry
var radiusSm = 6
var radius = 8
var radiusLg = 12
var radiusPill = 999

// Type
var fontUi = "Microsoft YaHei UI"
var fontMono = "Cascadia Mono, Consolas, Courier New"

// Motion
var fast = 120
var normal = 180

function loaderLabel(kind) {
    switch (kind) {
    case "fabric":
        return "Fabric"
    case "quilt":
        return "Quilt"
    case "forge":
        return "Forge"
    case "neoforge":
        return "NeoForge"
    default:
        return "原版"
    }
}

function statusTone(text) {
    if (!text)
        return ink3
    if (text.indexOf("失败") >= 0 || text.indexOf("无法") >= 0
            || text.indexOf("不存在") >= 0 || text.indexOf("无效") >= 0
            || text.indexOf("请先") >= 0 || text.indexOf("请选择") >= 0)
        return danger
    if (text.indexOf("完成") >= 0 || text.indexOf("已加载") >= 0)
        return green
    if (text.indexOf("正在") >= 0 || text.indexOf("准备") >= 0)
        return brand
    return ink2
}
