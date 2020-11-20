// This hard-wires things that are provded as TLAs to moo's
// omodel.jsonnet.  We only have one schema so it make the
// codegen/build easier to do.

local os = import "yamz-schema.jsonnet";
local oschema = import "oschema.jsonnet";
local ocpp = import "ocpp.jsonnet";
{
    lang: ocpp,

    // The path/namespace this model "lives" in
    path: oschema.listify("yamz"),
    nspre: oschema.prepath(self.path),

    // The path/namespace this model is assumed to be in.  It should
    // be a leading part of the path
    ctxpath: oschema.listify("yamz"),
    ctxpre: oschema.prepath(self.ctxpath),

    relpath: "",
    
    // Select out the types "in" this namespace.
    types: [t for t in os if oschema.isin(self.path, t.path)],
    all_types: os,

    // Reference full type by its FQN.
    byref: {[oschema.fqn(t)]:t for t in $.types},

    // Collect types by their schema class name
    byscn: {[tn]:[oschema.fqn(t) for t in $.types if t.schema == tn]
                   for tn in oschema.class_names},
    local extypref = [
        d for d in std.flattenArrays([t.deps for t in $.types])
          if !oschema.isin($.path,oschema.listify(d))],
    extrefs: std.uniq(std.sort([oschema.basepath(t) for t in extypref]))

}
