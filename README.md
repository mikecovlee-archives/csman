# The CovScript Manager Project

The man behind Covariant Script who maintains all versions, packages and lots of trivial stuffs for programmers.

**We call him `csman`.**

### 项目约定

- 语言标准：C++ 14（与 [CovScript](https://github.com/covscript/covscript) 主项目同步）
- 开发工具：任何支持 EditorConfig 的编辑器/IDE
- 开发模式：独立开发分支 -> PR -> Code Review -> 合并到主分支

### 开发计划

#### 平台
- [ ] Windows
    - [ ] MSVC [Experimental]
        - [ ] AMD64
        - [ ] i386
        - [ ] ARM64
        - [ ] ARM
    - [ ] MinGW-w64
        - [ ] AMD64 [Mainstream]
        - [ ] i386
- [ ] Linux
    - [ ] GCC
        - [ ] AMD64 [Mainstream]
        - [ ] i386
        - [ ] ARM
        - [ ] MIPS
- [ ] macOS
    - [ ] clang
        - [ ] AMD64 [Mainstream]

#### 功能

- [ ] 版本管理：支持安装多个 CovScript 版本，并支持设置默认版本
    - [ ] `csman install <latest | nightly | version> [--reinstall | --fix]`
    - [ ] `csman uninstall <all | version-regex> [--clean]`
    - [ ] `csman checkout <latest | version>`
    - [ ] `csman run [-v version] <command>`
    - [ ] `csman list`
- [ ] 扩展管理：全功能联网包管理器，支持分运行时版本进行管理
    - [ ] `csman install <package name> [--reinstall | --fix]`
    - [ ] `csman uninstall <package name(regex)> [--clean]`
    - [ ] `csman list`
- [ ] 配置管理：命令行配置管理器
    - [ ] `csman config set <key> <value>`
    - [ ] `csman config unset <key>`
    - [ ] `csman config get <key>`

#### CSMAN 源地址
- http://mirrors.covariant.cn/csman/
- http://install.covscript.org.cn/
- http://dist.csman.info/

#### CSMAN 源定义
- 根目录
    - csman.json
        - Version: 字符串，标示CSMAN标准版本
        - BaseUrl: 字符串，标示当前源基础地址
        - Platform: ["平台"...]
    - 平台(`Generic 或 OS_Compiler_Architecture`, 如`Win32_MSVC_AMD64`)
        - 注意：CovScript Package依赖标准版本号，CovScript Extension依赖二进制版本号，但这两者在书写依赖时也可以依赖特定版本的CovScript Runtime
        - csman.json
            - Version: {("VER":"运行时版本号" | "STD":"标准版本号" | "ABI":"二进制版本号")...}
            - Latest: [Version]
            - Nightly: [Version]
        - 运行时版本号(`VER_Master.Major.Minor.Revise`, 如`VER_2.3.3.3`)
            - csman.json
                - State: (Stable | Unstable | Preview) 字符串，指示版本状态
                - STD: 字符串，标示该运行时遵循的标准版本
                - ABI: 字符串，标示该运行时的二进制版本
                - RTM: 字符串，标示运行时环境的包名，一般为`cs.runtime`
                - DEV: 字符串，标示开发环境的包名，一般为`cs.develop`
                - ContentUrl: 字符串，可选，指定压缩包的存放地址
            - 发行包
                - CSMAN 包格式，打包成相应的ZIP压缩包，并改名为.csrtm后缀
            - 开发包
                - CSMAN 包格式，打包成相应的ZIP压缩包，并改名为.csdev后缀
        - 标准版本号(`STD_XXXXXX`，如`STD_200201`) 或 二进制版本号(`ABI_XXXXXX`，如`ABI_200201`)
            - 注意：所有的包必须依赖一个运行时版本
            - csman.json
                - {(包名(字符串):描述对象)...}
                - 描述对象格式:
                    - Description: 字符串，包的描述
                    - Author: 字符串，作者
                    - Verizon: ["版本号"...]
                    - Latest: [Version]
                    - Nightly: [Version]
                    - ContentUrl: 字符串，可选，指定压缩包的存放地址
            - 扩展包(`包名_版本号.cspkg`，如`cics.darwin_1.0.cspkg`)
                - CSMAN 包格式，打包成相应的ZIP压缩包，并改名为.cspkg后缀
#### CSMAN 包格式
- csman.json
    - State: (Stable | Unstable | Preview) 字符串，指示版本状态
    - Dependencies: {("包名":"latest | nightly | 版本号")...}
    - Contents: {("类型(BIN(可执行文件) | DEV(源代码头文件) | LIB(库文件) | CSP(CovScript包) | CSE(CovScript扩展) | DOC(文档))":"文件名 | 目录名")...}
- 内容: 需符合csman.json的描述
