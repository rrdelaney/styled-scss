module Path = {
  [@bs.deriving abstract]
  type t = {node: Babel_types.node};
};

[@bs.deriving abstract]
type traverser = {enter: Path.t => unit};

[@bs.module "@babel/traverse"]
external traverse : (Babel_types.node, traverser) => unit = "default";
