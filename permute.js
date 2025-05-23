import fs from "node:fs/promises"
import process from "node:process"
import subprocess from "node:child_process"

let fileName = process.argv[2] ?? "4k.c"
let text = await fs.readFile(fileName, "utf-8")

let opening = "([{".split("")
let closing = ")]}".split("")
let source = text.split(/\bG\( *([^, ]+) *, */)
let groups = new Map()

for (let i = 2 ; i < source.length ; i += 2) {
	
	let [group] = source[i - 1]
	let text = source[i]
	
	let m = 0
	for (let j = 0 ; j < text.length ; j++) {
		if (opening.includes(text[j])) m++
		if (closing.includes(text[j])) m--
		if (m < 0) {
			groups.set(group, groups.get(group) ?? [])
			groups.get(group).push(text.slice(0, j))
			source[i] = text.slice(j)
			break
		}
	}
}

let n = 1
await showAll([...groups])
await fs.writeFile(fileName, text)

async function showAll(array)
{
	if (array.length === 0) {
		await show()
		return
	}
	
	let permutations = []
	permute(permutations, array[0][1])
	for (let permutation of permutations) {
		groups.set(array[0][0], permutation)
		await showAll(array.slice(1))
	}
}

async function show()
{
	console.log(`possibility ${n++}`)
	let other = source.slice()
	let indices = new Map()
	for (let i = 1 ; i < other.length ; i += 2) {
		indices.set(other[i], (indices.get(other[i]) ?? -1) + 1)
		other[i] = `G(${other[i]}, ${groups.get(other[i])[indices.get(other[i])]}`
	}
	await fs.writeFile(fileName, other.join(""))
	subprocess.execFileSync("sh", [], {stdio: "inherit"})
}

// snagged from <https://stackoverflow.com/a/20871714>
function permute(results, array, memo = [])
{
	for (let i = 0 ; i < array.length ; i++) {
		let current = array.splice(i, 1)
		if (array.length === 0) results.push([...memo, ...current])
		permute(results, array.slice(), [...memo, ...current])
		array.splice(i, 0, current[0])
	}
}

