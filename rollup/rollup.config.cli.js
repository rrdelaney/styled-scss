import base from './rollup.config.base'

export default {
  ...base,
  input: 'src/cli.bs.js',
  output: {
    ...base.output,
    file: 'cli.js',
    banner: '#!/usr/bin/env node'
  }
}
