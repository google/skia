module.exports = {
  root: true,
  parser: '@typescript-eslint/parser',
  parserOptions: {
    tsconfigRootDir: __dirname,
    project: ['./tsconfig.json'],
  },
  plugins: [
    '@typescript-eslint',
  ],
  extends: [
    'airbnb-typescript',
    'plugin:@typescript-eslint/recommended-requiring-type-checking',
  ],
  settings: {
    react: { // silence the warning even though we don't use react
      "version": "latest",
    },
  },
  rules: {
    '@typescript-eslint/no-explicit-any': 'off',
    '@typescript-eslint/no-namespace': 'off',
    '@typescript-eslint/no-unsafe-call': 'off',
    '@typescript-eslint/no-unsafe-member-access': 'off',
    '@typescript-eslint/triple-slash-reference': 'off',
    '@typescript-eslint/indent': 'off',

    'import/prefer-default-export': 'off',
    'no-param-reassign': 'off',
    'no-underscore-dangle': 'off',
  }
};