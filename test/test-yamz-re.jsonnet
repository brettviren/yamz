// Run like:
//
// moo -D pass.model -M src \
//      validate --passfail --sequence \
//     -S pass.valid -s test/test-yamz-re.jsonnet \
//       test/test-yamz-re.jsonnet
//
// etc for fail.model and fail.valid to test failures.

local moo = import "moo.jsonnet";
local re = import "yamz-re.jsonnet";

local check = function(regex, examp) [{type:"string", pattern: regex}, examp];

local listify = function(checks) {
        valid: [one[0] for one in checks],
        model: [one[1] for one in checks],
};


local passes = [
    check(moo.re.zmq.tcp.uri, "tcp://127.0.0.1"),

    check(re.address.concrete.singular,"tcp://127.0.0.1?a=b&c=d"),
    check(re.address.concrete.singular,"ipc:///abs/pa-th/to.fifo?a=b&c=d"),
    check(re.address.concrete.singular,"ipc://re-la-ti-ve.fifo?a=b&c=d"),
    check(re.address.concrete.singular,"inproc://!@#$%^()_+\/.'~`"),
    check(re.address.concrete.singular,"inproc://!@#$%^()_+\/.'~?a=b&c=d"),

    check(re.address.ephemeral.singular,"tcp://*:*?a=b&c=d"),
    check(re.address.ephemeral.singular,"ipc://fill-me-*-in?a=b&c=d"),
    check(re.address.ephemeral.singular,"inproc://!@*#$%^()_+\/.'~`?a=b&c=d"),

    check(re.address.abstract.singular,"yamz://node/comp/port"),
    check(re.address.abstract.singular,"yamz://*/port?a=b&c=d"),
    check(re.address.abstract.singular,"yamz://node/*?a=b&c=d"),
    check(re.address.abstract.singular,"yamz://*/*/*?a=b&c=d"),
];

local fails = [
    check(re.address.concrete.singular,"tcp://*:*"),
    check(re.address.concrete.singular,"inproc://no-*-on"),
    check(re.address.abstract.singular,"tcp://127.0.0.1"),
];

{
    pass: listify(passes),
    fail: listify(fails),
}
