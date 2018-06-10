open Belt;

let typeFromPropType = t =>
  switch (t) {
  | "string" => ReasonablyTyped.DotTyped.String
  | "boolean" => ReasonablyTyped.DotTyped.Boolean
  | _ => ReasonablyTyped.DotTyped.String
  };

let generateReasonDefinition = (outputName, metadata: StyledMetadata.metadata) => {
  let components = Map.String.valuesToArray(metadata.components);

  let componentDeclarations =
    Array.map(components, component =>
      ReasonablyTyped.DotTyped.ReactComponent({
        name: ReasonablyTyped.DotTyped.Identifier(component.name),
        type_:
          ReasonablyTyped.DotTyped.Object({
            properties:
              Map.String.toArray(component.props)
              |. Array.map(((propName, propType)) =>
                   ReasonablyTyped.DotTyped.{
                     name:
                       ReasonablyTyped.DotTyped.Identifier(
                         Js.String.replace("$", "", propName),
                       ),
                     type_: typeFromPropType(propType),
                     optional: true,
                   }
                 ),
            typeParameters: [||],
            extends: None,
          }),
      })
    );

  let styledModule =
    ReasonablyTyped.DotTyped.ModuleDeclaration({
      name:
        ReasonablyTyped.DotTyped.Identifier(
          "./" ++ Node.Path.basename(outputName),
        ),
      declarations: componentDeclarations,
    });

  ReasonablyTyped.GenerateReason.compile(~skipFmt=false, styledModule);
};
