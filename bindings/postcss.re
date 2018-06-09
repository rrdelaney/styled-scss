module Options = {
  [@bs.deriving abstract]
  type t = {
    [@bs.as "from"] [@bs.optional]
    from_: string,
    [@bs.as "to"] [@bs.optional]
    to_: string,
  };
  let make = t;
};

module Internal = {
  module Raws = {
    type t;
    external toJson : t => Js.Json.t = "%identity";
    external fromJson : Js.Json.t => t = "%identity";
  };
  module Type = {
    let root = "root";
    let atrule = "atrule";
    let decl = "decl";
    let rule = "rule";
  };
  type node = {. "type": string};
  [@bs.module "postcss"] external parse : (string, Options.t) => node = "";
  [@bs.module "postcss"]
  external stringify : (node, string => unit) => unit = "";
};

type loc = {
  line: int,
  column: int,
};

type source = {
  start: option(loc),
  end_: option(loc),
};

type root = {
  source,
  raws: Internal.Raws.t,
  nodes: array(node),
}
and atrule = {
  source,
  raws: Internal.Raws.t,
  name: string,
  params: string,
  nodes: option(array(node)),
}
and decl = {
  source,
  raws: Internal.Raws.t,
  prop: string,
  value: string,
}
and rule = {
  source,
  raws: Internal.Raws.t,
  selector: string,
  mutable nodes: array(node),
}
and node =
  | Root(root)
  | Atrule(atrule)
  | Decl(decl)
  | Rule(rule)
  | Unknown(string);

module Decoder = {
  external nodeToJson : Internal.node => Js.Json.t = "%identity";
  let loc = json =>
    Json.Decode.{
      line: json |> field("line", int),
      column: json |> field("column", int),
    };
  let source = json =>
    Json.Decode.{
      start: json |> optional(field("start", loc)),
      end_: json |> optional(field("end", loc)),
    };
  let rec decoders = [
    (Internal.Type.root, root),
    (Internal.Type.atrule, atrule),
    (Internal.Type.decl, decl),
    (Internal.Type.rule, rule),
  ]
  and node = json => {
    let kind = json |> Json.Decode.field("type", Json.Decode.string);
    let decoder =
      try (List.assoc(kind, decoders)) {
      | _ => unknown
      };
    decoder(json);
  }
  and root = json =>
    Root(
      Json.Decode.{
        source: json |> field("source", source),
        nodes: json |> field("nodes", array(node)),
        raws: json |> field("raws", Internal.Raws.fromJson),
      },
    )
  and atrule = json =>
    Atrule(
      Json.Decode.{
        source: json |> field("source", source),
        name: json |> field("name", string),
        params: json |> field("params", string),
        nodes: json |> optional(field("nodes", array(node))),
        raws: json |> field("raws", Internal.Raws.fromJson),
      },
    )
  and decl = json =>
    Decl(
      Json.Decode.{
        source: json |> field("source", source),
        prop: json |> field("prop", string),
        value: json |> field("value", string),
        raws: json |> field("raws", Internal.Raws.fromJson),
      },
    )
  and rule = json =>
    Rule(
      Json.Decode.{
        source: json |> field("source", source),
        nodes: json |> field("nodes", array(node)),
        raws: json |> field("raws", Internal.Raws.fromJson),
        selector: json |> field("selector", string),
      },
    )
  and unknown = json => {
    let kind = Json.Decode.field("type", Json.Decode.string, json);
    Unknown(kind);
  };
  let decode = data => {
    let json = nodeToJson(data);
    node(json);
  };
};

module Encoder = {
  external jsonToNode : Js.Json.t => Internal.node = "%identity";
  let loc = (data: loc) =>
    Json.Encode.(
      object_([("line", int(data.line)), ("column", int(data.column))])
    );
  let source = (data: source) =>
    Json.Encode.(
      object_([
        ("start", nullable(loc, data.start)),
        ("end", nullable(loc, data.end_)),
      ])
    );
  let rec root = (data: root) =>
    Json.Encode.(
      object_([
        ("type", string(Internal.Type.root)),
        ("source", source(data.source)),
        ("nodes", array(node, data.nodes)),
        ("raws", Internal.Raws.toJson(data.raws)),
      ])
    )
  and atrule = (data: atrule) =>
    Json.Encode.(
      object_([
        ("type", string(Internal.Type.atrule)),
        ("source", source(data.source)),
        ("name", string(data.name)),
        ("params", string(data.params)),
        ("nodes", nullable(array(node), data.nodes)),
        ("raws", Internal.Raws.toJson(data.raws)),
      ])
    )
  and decl = (data: decl) =>
    Json.Encode.(
      object_([
        ("type", string(Internal.Type.decl)),
        ("source", source(data.source)),
        ("prop", string(data.prop)),
        ("value", string(data.value)),
        ("raws", Internal.Raws.toJson(data.raws)),
      ])
    )
  and rule = (data: rule) =>
    Json.Encode.(
      object_([
        ("type", string(Internal.Type.rule)),
        ("source", source(data.source)),
        ("selector", string(data.selector)),
        ("nodes", array(node, data.nodes)),
        ("raws", Internal.Raws.toJson(data.raws)),
      ])
    )
  and node =
    fun
    | Root(data) => root(data)
    | Atrule(data) => atrule(data)
    | Decl(data) => decl(data)
    | Rule(data) => rule(data)
    | Unknown(_) => raise(Not_found);
  let encode = json => {
    let data = node(json);
    jsonToNode(data);
  };
};

let parse = (source, options) => {
  let ast = Internal.parse(source, options);
  Decoder.decode(ast);
};

let stringify = ast => {
  let source = ref("");
  let data = Encoder.encode(ast);
  Internal.stringify(data, chunk => source := source^ ++ chunk);
  source^;
};
