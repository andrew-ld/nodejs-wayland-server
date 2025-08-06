import { readdirSync, statSync, copyFileSync } from 'fs';
import { join, basename } from 'path';

const installDir = join(import.meta.dirname, 'builddir', 'install');
const destDir = join(import.meta.dirname, 'artifacts');

function recursiveFindFiles(dir) {
    let results = [];
    try {
        const list = readdirSync(dir);
        for (const file of list) {
            const fullPath = join(dir, file);
            const stat = statSync(fullPath);
            if (stat && stat.isDirectory()) {
                results = results.concat(recursiveFindFiles(fullPath));
            } else {
                results.push(fullPath);
            }
        }
    } catch (err) {
        if (err.code !== 'ENOENT') throw err;
    }
    return results;
}

console.log(`Searching for artifacts in '${installDir}'...`);

const allFiles = recursiveFindFiles(installDir);

if (allFiles.length === 0) {
    console.error(`ERROR: Install directory '${installDir}' is empty or does not exist.`);
    console.error("Please ensure 'meson install' ran successfully.");
    process.exit(1);
}

const artifactsToCopy = allFiles.filter(file => {
    if (file.endsWith('.node')) {
        return true;
    }

    if (file.endsWith('.so')) {
        return true;
    }

    return false;
});

if (artifactsToCopy.length === 0) {
    console.error("ERROR: No '.node' or '.so' files found in the install directory.");
    console.error("The build may have failed to produce the final libraries.");
    process.exit(1);
}

console.log(`Found ${artifactsToCopy.length} artifacts to copy.`);

for (const srcPath of artifactsToCopy) {
    const destPath = join(destDir, basename(srcPath));
    try {
        copyFileSync(srcPath, destPath);
        console.log(`  -> Copied ${basename(srcPath)}`);
    } catch (err) {
        console.error(`Failed to copy ${srcPath} to ${destPath}`, err);
        process.exit(1);
    }
}

console.log('All artifacts copied successfully.');
