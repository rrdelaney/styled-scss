let getNodes =
  fun
  | Postcss.Root({nodes}) => nodes
  | Postcss.Atrule({nodes}) => nodes
  | Postcss.Rule({nodes}) => nodes
  | Postcss.Decl(_) => [||]
  | Postcss.Unknown(_) => [||];

type component = {
  name: string,
  props: Belt.Map.String.t(string),
};

type ifCondition = {condition: string};

type metadata = {
  components: Belt.Map.String.t(component),
  ifConditions: Belt.Map.String.t(ifCondition),
};

let emptyMetadata = {
  components: Belt.Map.String.empty,
  ifConditions: Belt.Map.String.empty,
};

let extractComponentName = str =>
  str |> Js.String.split("(") |. Array.get(0);

let extractProps = str => {
  let propsDef = str |. Js.String.split("(") |. Belt.Array.get(1);
  switch (propsDef) {
  | Some(props) =>
    props
    |. Js.String.split(",")
    |. Belt.Array.map(propWithType => {
         let propDef = Js.String.split(":", propWithType);
         (Js.String.trim(propDef[0]), Js.String.trim(propDef[1]));
       })
    |. Belt.Map.String.fromArray
  | None => Belt.Map.String.empty
  };
};

let componentSelector = params =>
  "." ++ extractComponentName(params) ++ "Component";

let addComponent = (params, metadata) => {
  ...metadata,
  components:
    Belt.Map.String.set(
      metadata.components,
      componentSelector(params),
      {name: extractComponentName(params), props: extractProps(params)},
    ),
};

let ifConditionSelector = metadata =>
  ".If" ++ (metadata.ifConditions |. Belt.Map.String.size |. string_of_int);

let addIfCondition = (params, metadata) => {
  ...metadata,
  ifConditions:
    Belt.Map.String.set(
      metadata.ifConditions,
      ifConditionSelector(metadata),
      {condition: params},
    ),
};

let rec extract = (~replaceSelf=_newNode => (), ~metadata=emptyMetadata, node) =>
  switch (node) {
  | Postcss.Root(root) =>
    root.nodes
    |. Belt.Array.mapWithIndex((index, node) => (index, node))
    |. Belt.Array.reduce(
         metadata,
         (metadata, (index, node)) => {
           let replaceNode = newNode => root.nodes[index] = newNode;
           extract(~replaceSelf=replaceNode, ~metadata, node);
         },
       )
  | Postcss.Atrule(atrule) =>
    let metadata =
      if (atrule.name == "component") {
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
      } else if (atrule.name == "else") {
        metadata;
      } else {
        metadata;
      };
    metadata;
  | _ => metadata
  }
and extractNodes = (nodeArray, metadata) =>
  nodeArray
  |. Belt.Array.mapWithIndex((index, node) => (index, node))
  |. Belt.Array.reduce(
       metadata,
       (metadata, (index, node)) => {
         let replaceNode = newNode => nodeArray[index] = newNode;
         extract(~replaceSelf=replaceNode, ~metadata, node);
       },
     );
