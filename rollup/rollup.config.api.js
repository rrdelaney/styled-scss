import base from './rollup.config.base'

export default {
  ...base,
  input: 'src/api.bs.js',
  output: {
    ...base.output,
    file: 'index.js'
  }
}
