/***
 * Nests rule nodes as far as they can be nested, following Sass nesting.
 *
 * Note that this mutates the AST in place, and also returns a new node array.
 * It's a little weird, but it works ok.
 */
let rec foldNodes = nodes => {
  /* Reference to the last selector that only had one element. */
  let lastRoot = ref(None);
  Belt.Array.keepMap(nodes, node =>
    switch (node) {
    /* Finds a rule with a selector with no spaces. */
    | Postcss.Rule({selector} as rule)
        when ! Js.String.includes(" ", selector) =>
      /* Process the old last node if it exists, and store the new folded nodes. */
      Postcss.(
        switch (lastRoot^) {
        | Some(n) => n.nodes = foldNodes(n.nodes)
        | None => ()
        }
      );
      /* This node is the new last node. */
      lastRoot := Some(rule);
      /* Keep this node. */
      Some(node);
    /* A CSS rule that has might be nested. */
    | Postcss.Rule({selector} as rule) =>
      /* If there is a last root, check to see if this selector can be nested under it. */
      switch (lastRoot^) {
      | Some(lastRule) when Js.String.startsWith(lastRule.selector, selector) =>
        /* If this selector can be nested, trim the old selector and nest this node under it. */
        let _ =
          Js.Array.push(
            Postcss.Rule({
              ...rule,
              selector:
                selector
                |> Js.String.replace(lastRule.selector, "")
                |> Js.String.trim,
            }),
            lastRule.nodes,
          );
        None;
      | _ => Some(node)
      }
    | _ => Some(node)
    }
  );
};


/***
 * Folds up any CSS rules that can be nested under each other. Returns a new AST, but
 * also mutates the old one in place.
 */
let nestComponentRules = ast =>
  switch (ast) {
  | Postcss.Root({nodes} as root) =>
    Postcss.Root({...root, nodes: foldNodes(nodes)})
  | node => node
  };
