local moo = import "moo.jsonnet";
local re = import "yamz-re.jsonnet";

local mks = function(regex) {type:"string", pattern: regex};

{
    valid: mks(moo.re.tcp),
    data: "ssstcp://127.0.0.1",
}
