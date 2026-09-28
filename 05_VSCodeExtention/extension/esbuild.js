const esbuild = require('esbuild');
const watch = process.argv.includes('--watch');

async function main() {
  const ctx = await esbuild.context({
    // buildShaders.js はビルド用 bat が VSCode 本体(Node として)で呼ぶ(DESIGN.md 9.2 章)
    entryPoints: { extension: 'src/extension.ts', buildShaders: 'src/shader/buildShaders.ts' },
    bundle: true,
    format: 'cjs',
    platform: 'node',
    target: 'node18',
    outdir: 'dist',
    external: ['vscode'],
    sourcemap: true,
    minify: false,
    logLevel: 'info',
  });
  if (watch) {
    await ctx.watch();
  } else {
    await ctx.rebuild();
    await ctx.dispose();
  }
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
