package main

import "fmt"

func getMostLeftIndex(target int, nums []int) int {
	left := 0
	right := len(nums) - 1
	for left < right {
		mid := left + (right-left)/2
		if nums[mid] < target {
			left = mid + 1
		} else {
			right = mid
		}
	}
	if nums[left] == target {
		return left
	}
	return -1

}

func getMostLeftIndex2(target int, nums []int) int {
	left := 0
	right := len(nums) - 1
	mid := left + (right-left)/2
	for left <= right {
		mid = left + (right-left)/2
		if nums[mid] < target {
			left = mid + 1
		} else if nums[mid] > target {
			right = mid - 1
		} else {
			right = mid - 1
		}
	}
	if nums[mid] == target {
		return mid
	}
	return -1
}

type getIndexFunc func(int, []int) int

func test(getIndex getIndexFunc, target int, data []int) {
	index := getIndex(target, data)
	if index >= 0 {
		fmt.Println("index is ", index)
	} else {
		fmt.Println("not found")
	}

}

func main() {
	data := []int{1, 2, 3, 3, 4, 5, 7}

	test(getMostLeftIndex, 3, data[:1])
	test(getMostLeftIndex, 3, data[2:3])
	test(getMostLeftIndex, 3, data[:3])
	test(getMostLeftIndex, 3, data[3:])
	test(getMostLeftIndex, 3, data[:])

	test(getMostLeftIndex2, 3, data[:1])
	test(getMostLeftIndex2, 3, data[2:3])
	test(getMostLeftIndex2, 3, data[:3])
	test(getMostLeftIndex2, 3, data[3:])
	test(getMostLeftIndex2, 3, data[:])
}
