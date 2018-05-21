open Belt;


/***
 * Retrieves any child nodes from a Postcss AST node.
 */
let getNodes =
  fun
  | Postcss.Root({nodes}) => nodes
  | Postcss.Atrule({nodes}) => nodes
  | Postcss.Rule({nodes}) => nodes
  | Postcss.Decl(_) => [||]
  | Postcss.Unknown(_) => [||];


/***
 * Metadata about a component declared in the input Sass file.
 */
type component = {
  name: string,
  props: Belt.Map.String.t(string),
};


/***
 * Metadata about an if condition declared in the input Sass file.
 */
type ifCondition = {condition: string};


/***
 * All metadata extracted from an input file. Contains components,
 * if conditions, etc.
 */
type metadata = {
  components: Belt.Map.String.t(component),
  ifConditions: Belt.Map.String.t(ifCondition),
};


/***
 * Empty metadata to start with.
 */
let emptyMetadata = {
  components: Map.String.empty,
  ifConditions: Map.String.empty,
};


/***
 * Extracts a component name from a string like "PrettyButton($prop: int)".
 */
let extractComponentName = str =>
  str |> Js.String.split("(") |. Array.getExn(0);


/***
 * Extracts a prop definitions from a string like "PrettyButton($prop: int)".
 */
let extractProps = str => {
  let propsDef = str |. Js.String.split("(") |. Array.get(1);
  switch (propsDef) {
  | Some(props) =>
    props
    |. Js.String.split(",")
    |. Array.map(propWithType => {
         let propDef = Js.String.split(":", propWithType);
         let propName = Array.getExn(propDef, 0);
         let propType = Array.getExn(propDef, 1);
         (Js.String.trim(propName), Js.String.trim(propType));
       })
    |. Map.String.fromArray
  | None => Map.String.empty
  };
};


/***
 * Creates a selector for a component declared in an atrule.
 */
let componentSelector = params =>
  "." ++ extractComponentName(params) ++ "Component";


/***
 * Adds a component from an atrule to the collected metadata.
 */
let addComponent = (params, metadata) => {
  ...metadata,
  components:
    Belt.Map.String.set(
      metadata.components,
      componentSelector(params),
      {name: extractComponentName(params), props: extractProps(params)},
    ),
};


/***
 * Creates a selector for an if condition declared in an atrule.
 */
let ifConditionSelector = metadata =>
  ".If" ++ (metadata.ifConditions |. Belt.Map.String.size |. string_of_int);


/***
 * Adds an if condition from an atrule to the collected metadata.
 */
let addIfCondition = (params, metadata) => {
  ...metadata,
  ifConditions:
    Map.String.set(
      metadata.ifConditions,
      ifConditionSelector(metadata),
      {condition: params},
    ),
};


/***
 * Extracts metadata from a Postcss AST and replaces dynamic atrule nodes with
 * static class names. Note that the class name replacement happens in-place,
 * and this method mutates the AST passed in.
 */
let rec extract = (~replaceSelf=_newNode => (), ~metadata=emptyMetadata, node) =>
  switch (node) {
  | Postcss.Root(root) =>
    root.nodes
    |. Array.mapWithIndex((index, node) => (index, node))
    |. Array.reduce(
         metadata,
         (metadata, (index, node)) => {
           let replaceNode = Array.setExn(root.nodes, index);
           extract(~replaceSelf=replaceNode, ~metadata, node);
         },
       )
  | Postcss.Atrule(atrule) =>
    let metadata =
      if (atrule.name == "component") {
        /* Component at rules are replaced with a component selector, but same nodes. */
        let newRule =
          Postcss.Rule({
            source: atrule.source,
            raws: atrule.raws,
            nodes: atrule.nodes,
            selector: componentSelector(atrule.params),
          });
        let metadata =
          extractNodes(
            getNodes(newRule),
            addComponent(atrule.params, metadata),
          );
        replaceSelf(newRule);
        metadata;
      } else if (atrule.name == "if") {
        /* If statements are replaced with a special selector too. */
        let newRule =
          Postcss.Rule({
            source: atrule.source,
            raws: atrule.raws,
            nodes: atrule.nodes,
            selector: ifConditionSelector(metadata),
          });
        let metadata =
          extractNodes(
            getNodes(newRule),
            addIfCondition(atrule.params, metadata),
          );
        replaceSelf(newRule);
        metadata;
      } else {
        metadata;
      };
    metadata;
  | _ => metadata
  }
/***
 * Runs `extract` over an array of nodes.
 */
and extractNodes = (nodeArray, metadata) =>
  nodeArray
  |. Array.mapWithIndex((index, node) => (index, node))
  |. Array.reduce(
       metadata,
       (metadata, (index, node)) => {
         let replaceNode = Array.setExn(nodeArray, index);
         extract(~replaceSelf=replaceNode, ~metadata, node);
       },
     );
