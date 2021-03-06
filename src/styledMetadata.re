open Belt;

/** Retrieves any child nodes from a Postcss AST node. */
let getNodes =
  fun
  | Postcss.Root({nodes}) => nodes
  | Postcss.Atrule({nodes: None}) => [||]
  | Postcss.Atrule({nodes: Some(nodes)}) => nodes
  | Postcss.Rule({nodes}) => nodes
  | Postcss.Decl(_) => [||]
  | Postcss.Unknown(_) => [||];

/** Metadata about a component declared in the input Sass file. */
type component = {
  element: string,
  name: string,
  props: Map.String.t(string),
};

/** Metadata about an if condition declared in the input Sass file. */
type ifCondition = {condition: string};

/** All metadata extracted from an input file. Contains components, if
conditions, etc. */
type metadata = {
  components: Map.String.t(component),
  ifConditions: Map.String.t(ifCondition),
};

/** Empty metadata to start with. */
let emptyMetadata = {
  components: Map.String.empty,
  ifConditions: Map.String.empty,
};

/** Extracts a component name from a string like "PrettyButton($prop: int)". */
let extractComponentName = str =>
  str |> Js.String.split("(") |. Array.getExn(0);

/** Extracts a prop definitions from a string like "PrettyButton($prop: int)". */
let extractProps = str => {
  let propsDef = str |> Js.String.split("(") |. Array.get(1);
  switch (propsDef) {
  | Some(props) =>
    props
    |> Js.String.replace(")", "")
    |> Js.String.split(",")
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

/** Determines if an atrule is trying to generate a styled component. */
let isComponentAtrule = Js.String.startsWith("styled.");

/** Returns the type of element to be generated for the component. */
let componentElementType = Js.String.replace("styled.", "");

/** Creates a selector for a component declared in an atrule. */
let componentSelector = params =>
  "." ++ extractComponentName(params) ++ "Component";

/** Creates a new metadata object for a component based on atrule params. */
let createComponent = (rule, params) => {
  name: extractComponentName(params),
  props: extractProps(params),
  element: componentElementType(rule),
};

/** Adds a component from an atrule to the collected metadata. */
let addComponent = (rule, params, metadata) => {
  ...metadata,
  components:
    Belt.Map.String.set(
      metadata.components,
      componentSelector(params),
      createComponent(rule, params),
    ),
};

/** Creates a selector for an if condition declared in an atrule. */
let ifConditionSelector = metadata =>
  ".If" ++ (metadata.ifConditions |. Belt.Map.String.size |. string_of_int);

/** Adds an if condition from an atrule to the collected metadata. */
let addIfCondition = (params, metadata) => {
  ...metadata,
  ifConditions:
    Map.String.set(
      metadata.ifConditions,
      ifConditionSelector(metadata),
      {condition: params},
    ),
};

/** Determines if a given `if` condition is dynamic, or should be statically
compiled by Sass. */
let isDynamicCondition = (component, atrule: Postcss.atrule) =>
  switch (component) {
  | Some({props}) when atrule.name == "if" =>
    Map.String.some(props, (key, _value) =>
      Js.String.includes(key, atrule.params)
    )
  | _ => false
  };

/** Replaces all usages of a prop name with an escaped name. */
let replacePropName = (ruleValue, propName) => {
  let replacerRe = Js.Re.fromStringWithFlags(~flags="g", "\\" ++ propName);
  let rawPropName = Js.String.replace("$", "", propName);
  let replacedVal = "_local_var_" ++ rawPropName ++ "_local_";
  Js.String.replaceByRe(replacerRe, replacedVal, ruleValue);
};

/** Extracts metadata from a Postcss AST and replaces dynamic atrule nodes with
static class names. Note that the class name replacement happens in-place,
and this method mutates the AST passed in. */
let rec extract =
        (
          ~replaceSelf=_newNode => (),
          ~currentComponent=None,
          ~metadata=emptyMetadata,
          node,
        ) =>
  switch (node) {
  /* At the root start to recurse into Atrules and Rules. */
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

  | Postcss.Atrule({nodes: None}) => metadata

  /* Component at rules are replaced with a component selector, but same nodes. */
  | Postcss.Atrule({nodes: Some(nodes)} as atrule)
      when isComponentAtrule(atrule.name) =>
    let newRule =
      Postcss.Rule({
        source: atrule.source,
        raws: atrule.raws,
        nodes,
        selector: componentSelector(atrule.params),
      });
    let metadata =
      extractNodes(
        ~nodes=getNodes(newRule),
        ~metadata=addComponent(atrule.name, atrule.params, metadata),
        ~component=Some(createComponent(atrule.name, atrule.params)),
      );
    replaceSelf(newRule);
    metadata;

  /* If statements are replaced with a special selector too. */
  | Postcss.Atrule({nodes: Some(nodes)} as atrule)
      when isDynamicCondition(currentComponent, atrule) =>
    let newRule =
      Postcss.Rule({
        source: atrule.source,
        raws: atrule.raws,
        nodes,
        selector: ifConditionSelector(metadata),
      });
    let metadata =
      extractNodes(
        ~nodes=getNodes(newRule),
        ~metadata=addIfCondition(atrule.params, metadata),
        ~component=currentComponent,
      );
    replaceSelf(newRule);
    metadata;

  /* Anything else that has child nodes should just recurse down into them. */
  | Postcss.Atrule({nodes: Some(nodes)})
  | Postcss.Rule({nodes}) =>
    extractNodes(~nodes, ~metadata, ~component=currentComponent)

  /* Terminal things like declarations end the recursion. */
  | Postcss.Decl({value} as decl) when Option.isSome(currentComponent) =>
    let {props} = Option.getExn(currentComponent);

    let newValue =
      Map.String.reduce(props, value, (ruleValue, propName, _propType) =>
        replacePropName(ruleValue, propName)
      );
    let newRule = Postcss.Decl({...decl, value: newValue});
    replaceSelf(newRule);

    metadata;

  | _ => metadata
  }
/** Runs `extract` over an array of nodes. */
and extractNodes = (~nodes, ~metadata, ~component) =>
  nodes
  |. Array.mapWithIndex((index, node) => (index, node))
  |. Array.reduce(
       metadata,
       (metadata, (index, node)) => {
         let replaceNode = Array.setExn(nodes, index);
         extract(
           ~replaceSelf=replaceNode,
           ~metadata,
           ~currentComponent=component,
           node,
         );
       },
     );
