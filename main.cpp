#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

class DianmaConverter {
private:
    std::unordered_map<std::string, std::string> chineseToCode;  // 中文->电码
    std::unordered_map<std::string, std::string> codeToChinese;  // 电码->中文
    std::unordered_map<std::string, std::string> letterToChinese; // 字母电码->中文
    
    // 从文件加载电码数据
    bool loadDataFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return false;
        }
        
        std::string line;
        bool isDataSection = false;
        int lineCount = 0;
        
        while (std::getline(file, line)) {
            lineCount++;
            
            // 跳过文件头
            if (lineCount <= 3) continue;
            
            // 检查是否是空行
            if (line.empty()) continue;
            
            // 检查是否是数据行（包含电码和字符）
            if (line.length() > 10) {
                // 解析行数据
                std::istringstream iss(line);
                std::string digitalCode, letterCode, chineseChar, name, originalNote, comment;
                
                // 尝试解析每一列
                if (iss >> digitalCode >> letterCode >> chineseChar) {
                    // 只处理有中文字符的行
                    if (!chineseChar.empty() && chineseChar != "UCS") {
                        // 清理中文字符（移除可能的多余字符）
                        chineseChar = chineseChar.substr(0, 3); // 通常中文字符是1-3个字节
                        
                        // 存储映射关系
                        chineseToCode[chineseChar] = digitalCode;
                        codeToChinese[digitalCode] = chineseChar;
                        letterToChinese[letterCode] = chineseChar;
                    }
                }
            }
        }
        
        file.close();
        std::cout << "加载了 " << chineseToCode.size() << " 个字符的映射" << std::endl;
        return true;
    }
    
    // 从程序内嵌数据加载（如果文件不可用）
    void loadEmbeddedData() {
        // 这里可以添加一些常见字符的内置数据
        // 为了简化，我只添加几个示例
        addMapping("0001", "AAB", "一");
        addMapping("0002", "AAC", "丁");
        addMapping("0003", "AAD", "七");
        addMapping("0004", "AAE", "丈");
        addMapping("0005", "AAF", "三");
        addMapping("0006", "AAG", "上");
        addMapping("0007", "AAH", "下");
        addMapping("0008", "AAI", "不");
        addMapping("0009", "AAJ", "丐");
        addMapping("0010", "AAK", "丑");
        // 可以继续添加更多...
        
        std::cout << "使用内置数据，加载了 " << chineseToCode.size() << " 个字符的映射" << std::endl;
    }
    
    void addMapping(const std::string& digitalCode, const std::string& letterCode, const std::string& chineseChar) {
        chineseToCode[chineseChar] = digitalCode;
        codeToChinese[digitalCode] = chineseChar;
        letterToChinese[letterCode] = chineseChar;
    }
    
public:
    DianmaConverter(const std::string& filename = "") {
        if (!filename.empty()) {
            if (!loadDataFromFile(filename)) {
                std::cout << "使用内置数据..." << std::endl;
                loadEmbeddedData();
            }
        } else {
            loadEmbeddedData();
        }
    }
    
    // 中文转数字电码
    std::string chineseToDigitalCode(const std::string& chinese) {
        std::string result;
        for (size_t i = 0; i < chinese.length(); ) {
            // 处理UTF-8字符（中文字符通常是3个字节）
            std::string chineseChar;
            if ((chinese[i] & 0xF0) == 0xE0 && i + 2 < chinese.length()) {
                // UTF-8 3字节字符（大多数中文）
                chineseChar = chinese.substr(i, 3);
                i += 3;
            } else if ((chinese[i] & 0x80) == 0) {
                // ASCII字符
                chineseChar = chinese.substr(i, 1);
                i += 1;
            } else {
                // 其他情况，跳过
                i++;
                continue;
            }
            
            auto it = chineseToCode.find(chineseChar);
            if (it != chineseToCode.end()) {
                result += it->second + " ";
            } else {
                result += "???? ";
            }
        }
        
        if (!result.empty() && result.back() == ' ') {
            result.pop_back();
        }
        
        return result;
    }
    
    // 中文转字母电码（通过数字电码转换）
    std::string chineseToLetterCode(const std::string& chinese) {
        std::string result;
        // 由于我们只有字母电码->中文的映射，没有中文->字母电码的直接映射
        // 这里需要先获取数字电码，然后查找对应的字母电码
        // 为了简化，我们只返回数字电码，并提示用户
        result = chineseToDigitalCode(chinese);
        return result + " (字母电码功能需要完整数据库)";
    }
    
    // 数字电码转中文
    std::string digitalCodeToChinese(const std::string& codeStr) {
        std::istringstream iss(codeStr);
        std::string code;
        std::string result;
        
        while (iss >> code) {
            // 确保电码是4位数字
            if (code.length() < 4) {
                code = std::string(4 - code.length(), '0') + code;
            }
            
            auto it = codeToChinese.find(code);
            if (it != codeToChinese.end()) {
                result += it->second;
            } else {
                result += "？";
            }
        }
        
        return result;
    }
    
    // 字母电码转中文
    std::string letterCodeToChinese(const std::string& codeStr) {
        std::istringstream iss(codeStr);
        std::string code;
        std::string result;
        
        while (iss >> code) {
            // 转换为大写
            std::transform(code.begin(), code.end(), code.begin(), ::toupper);
            
            auto it = letterToChinese.find(code);
            if (it != letterToChinese.end()) {
                result += it->second;
            } else {
                result += "？";
            }
        }
        
        return result;
    }
    
    // 显示帮助信息
    void showHelp() {
        std::cout << "\n=== 电码转换程序使用说明 ===\n";
        std::cout << "1. 中文转电码: 输入中文文本\n";
        std::cout << "2. 电码转中文: 输入4位数字电码(用空格分隔多个电码)\n";
        std::cout << "3. 字母电码转中文: 输入3位字母电码(如AAB AAC，用空格分隔)\n";
        std::cout << "4. 帮助: help\n";
        std::cout << "5. 退出: exit 或 quit\n";
        std::cout << "===========================\n\n";
    }
    
    // 运行交互式程序
    void runInteractive() {
        std::cout << "欢迎使用电码转换程序！\n";
        std::cout << "输入 'help' 查看帮助，'exit' 退出程序\n\n";
        
        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);
            
            if (input == "exit" || input == "quit") {
                std::cout << "再见！\n";
                break;
            } else if (input == "help") {
                showHelp();
            } else if (input.empty()) {
                continue;
            } else {
                // 判断输入类型
                if (isChineseText(input)) {
                    // 中文转电码
                    std::string digitalCode = chineseToDigitalCode(input);
                    std::cout << "数字电码: " << digitalCode << std::endl;
                } else if (isDigitalCode(input)) {
                    // 数字电码转中文
                    std::string chinese = digitalCodeToChinese(input);
                    std::cout << "中文: " << chinese << std::endl;
                } else if (isLetterCode(input)) {
                    // 字母电码转中文
                    std::string chinese = letterCodeToChinese(input);
                    std::cout << "中文: " << chinese << std::endl;
                } else {
                    std::cout << "无法识别输入，请输入中文、4位数字电码或3位字母电码\n";
                }
            }
        }
    }
    
private:
    // 判断是否为中文字符串
    bool isChineseText(const std::string& str) {
        if (str.empty()) return false;
        
        // 检查是否包含中文字符（UTF-8）
        for (size_t i = 0; i < str.length(); ) {
            unsigned char c = str[i];
            if (c >= 0xE0 && c <= 0xEF && i + 2 < str.length()) {
                // 可能是中文字符
                return true;
            } else if (c < 0x80) {
                // ASCII字符，继续检查
                i++;
            } else {
                // 其他情况
                i++;
            }
        }
        
        return false;
    }
    
    // 判断是否为数字电码字符串
    bool isDigitalCode(const std::string& str) {
        if (str.empty()) return false;
        
        std::istringstream iss(str);
        std::string code;
        int count = 0;
        
        while (iss >> code) {
            count++;
            // 检查是否为纯数字
            for (char c : code) {
                if (!std::isdigit(c)) {
                    return false;
                }
            }
            
            // 检查长度是否为4位（或少于4位）
            if (code.length() > 4) {
                return false;
            }
        }
        
        return count > 0;
    }
    
    // 判断是否为字母电码字符串
    bool isLetterCode(const std::string& str) {
        if (str.empty()) return false;
        
        std::istringstream iss(str);
        std::string code;
        int count = 0;
        
        while (iss >> code) {
            count++;
            // 检查是否为3位大写字母
            if (code.length() != 3) {
                return false;
            }
            
            for (char c : code) {
                if (!std::isalpha(c)) {
                    return false;
                }
            }
        }
        
        return count > 0;
    }
};

// 文件处理工具函数
bool saveConversionToFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return false;
    }
    
    file << content << std::endl;
    file.close();
    return true;
}

int main(int argc, char* argv[]) {
    // 设置控制台编码（Windows）
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif
    
    std::cout << "=== 中文电码转换程序 ===\n";
    
    DianmaConverter converter;
    
    // 检查是否有命令行参数
    if (argc > 1) {
        std::string mode = argv[1];
        
        if (mode == "-f" && argc > 2) {
            // 从文件加载数据
            DianmaConverter fileConverter(argv[2]);
            fileConverter.runInteractive();
        } else if (mode == "-c" && argc > 2) {
            // 命令行模式：中文转电码
            std::string chinese = argv[2];
            for (int i = 3; i < argc; i++) {
                chinese += " " + std::string(argv[i]);
            }
            std::string code = converter.chineseToDigitalCode(chinese);
            std::cout << "中文: " << chinese << std::endl;
            std::cout << "数字电码: " << code << std::endl;
        } else if (mode == "-d" && argc > 2) {
            // 命令行模式：数字电码转中文
            std::string code = argv[2];
            for (int i = 3; i < argc; i++) {
                code += " " + std::string(argv[i]);
            }
            std::string chinese = converter.digitalCodeToChinese(code);
            std::cout << "数字电码: " << code << std::endl;
            std::cout << "中文: " << chinese << std::endl;
        } else if (mode == "-l" && argc > 2) {
            // 命令行模式：字母电码转中文
            std::string code = argv[2];
            for (int i = 3; i < argc; i++) {
                code += " " + std::string(argv[i]);
            }
            std::string chinese = converter.letterCodeToChinese(code);
            std::cout << "字母电码: " << code << std::endl;
            std::cout << "中文: " << chinese << std::endl;
        } else {
            std::cout << "使用方法:\n";
            std::cout << "  交互模式: " << argv[0] << "\n";
            std::cout << "  从文件加载数据: " << argv[0] << " -f 文件名.txt\n";
            std::cout << "  中文转电码: " << argv[0] << " -c \"中文文本\"\n";
            std::cout << "  电码转中文: " << argv[0] << " -d \"0001 0002 0003\"\n";
            std::cout << "  字母电码转中文: " << argv[0] << " -l \"AAB AAC AAD\"\n";
        }
    } else {
        // 交互模式
        converter.runInteractive();
    }
    
    return 0;
}
