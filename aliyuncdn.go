package main

import (
    "fmt"
    "sort"
)

// 编码表元素的类型
type CodeTableEntry struct {
    num int64
    str string
}
type CodeTableEntryArray []CodeTableEntry

// 编码表元素的比较函数
func (arr CodeTableEntryArray) Len() int {
    return len(arr)
}
func (arr CodeTableEntryArray) Less(i, j int) bool {
    if arr[i].num == arr[j].num {
        return arr[i].str < arr[j].str
    }
    return arr[i].num < arr[j].num
}
func (arr CodeTableEntryArray) Swap(i, j int) {
    arr[i].num, arr[j].num = arr[j].num, arr[i].num
    arr[i].str, arr[j].str = arr[j].str, arr[i].str
}

func main() {
    table := map[string]int {
        "a":3,"b": 43,"c": 7,"d" :31,"e": 47,"f": 53,"g": 59,
        "h": 61,"i": 19,"j": 11,"k": 67,"l": 5,"m": 41,"n": 37,
        "o": 17,"p": 71,"q": 73,"r": 79,"s": 83,"t": 13,"u": 29,
        "v": 89,"w": 2,"x": 97,"y": 23,"z": 101 }
    formats := [4]string { "%se%s%s%s%se", "%s%s", "%s%s%s%s", "%s%si%s%snc%sn" }
    codes := [3]int64 { 5025370, 22763, 13540483 }

    words := Aliyuncdn(table, formats, codes)
    for _, word := range words {
        fmt.Print(word + " ")
    }
    fmt.Println()
}

func Aliyuncdn(table map[string]int, formats [4]string, codes [3]int64) (words []string) {
    // 排序
    var factorStrMap CodeTableEntryArray
    for k, v := range table {
        factorStrMap = append(factorStrMap, CodeTableEntry{int64(v), k})
    }
    sort.Sort(CodeTableEntryArray(factorStrMap))

    // 解析codes
    for i, code := range codes {
        factorStrs := ParseCode(factorStrMap, code)
        words = append(words, fmt.Sprintf(formats[i], factorStrs...))
    }

    // 解析不定code
    for x := 31945955; x <= 31945975; x++ {
        factorStrs := ParseCode(factorStrMap, int64(x))
        if factorStrs == nil {
            continue
        }
        word := fmt.Sprintf(formats[3], factorStrs...)
        words = append(words, word)
        break
    }
    return words
}

// 对code的因式分解, 并获取质因子对应的字符串
func ParseCode(factors CodeTableEntryArray, code int64) (factorStrs []interface{}) {
    const BASE_PRIME_FACTOR int64 = 103
    code /= BASE_PRIME_FACTOR

    for _, factor := range factors { // factor是从小到大
        if factor.num > code {
            break
        }
        if factor.num * (code / factor.num) == code {
            factorStrs = append(factorStrs, factor.str)
            code /= factor.num
        }
    }
    if code != 1 {
        return nil
    }
    return factorStrs
}
