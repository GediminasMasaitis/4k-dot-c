import fs from "node:fs/promises"
import process from "node:process"

let text = await fs.readFile(process.argv[2] ?? "4k.c", "utf-8")

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
showAll([...groups])

function showAll(array)
{
	if (array.length === 0) {
		show()
		return
	}
	
	let permutations = []
	permute(permutations, array[0][1])
	for (let permutation of permutations) {
		groups.set(array[0][0], permutation)
		showAll(array.slice(1))
	}
}

function show()
{
	console.log(`/ / / / / POSSIBILITY ${n++} / / / / /`)
	console.log("")
	let other = source.slice()
	let indices = new Map()
	for (let i = 1 ; i < other.length ; i += 2) {
		indices.set(other[i], (indices.get(other[i]) ?? -1) + 1)
		other[i] = `G(${other[i]}, ${groups.get(other[i])[indices.get(other[i])]})`
	}
	console.log(other.join(""))
	console.log("")
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
