local moo = import "moo.jsonnet";
local yschema = import "yamz-schema.jsonnet";
local json_schema_id = "https://brettviren.github.io/yamz/test/test-yamz-cluster-valid.json";
local jschema = moo.jschema.convert(yschema, json_schema_id);
jschema { "$ref": "#/definitions/TestJobCfg" }

