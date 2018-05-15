type flags;

[@bs.deriving abstract]
type meow = {
  input: array(string),
  flags,
};

[@bs.module] external make : string => meow = "meow";
