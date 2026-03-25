<div align="center">

# 🎨 L4D2 Materials Helper

**《求生之路2》材质自动化处理工具**

[![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.10-green.svg?style=flat-square&logo=qt)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg?style=flat-square&logo=windows)](https://www.microsoft.com/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/Glyer1/L4D2MaterialsHelper?style=flat-square&logo=github)](https://github.com/Glyer1/L4D2MaterialsHelper/stargazers)

</div>

---

## 📌 项目背景

作为 L4D2 Mod 作者，每次制作角色材质都需要：

- 🔄 手动将几十张 PNG 转换为 VTF 格式
- ✍️ 手写 VMT 材质文件
- 🖼️ 配置 Lightwarp、Bumpmap 等贴图路径
- 🎨 为每个颜色变体重复上述操作
- 📦 最后打包成 VPK

这个过程**耗时且容易出错**。本工具将上述流程**一键自动化**，将材质处理时间从 **30 分钟缩短至 10 分钟**。

---

## ✨ 核心功能

| 功能模块              | 说明                                                         |
| --------------------- | ------------------------------------------------------------ |
| 🎯 **智能材质处理**    | 自动识别普通材质和动图材质（支持 `@` 分隔多帧），根据差分名自动处理 Alpha 通道 |
| 🧩 **组件化配置**      | 创建组件（如鞋子、裙子），为每个组件配置材质和颜色变体       |
| 🔧 **完整流水线**      | PNG/TGA → 预处理 → VTF → VMT → VPK，支持跳过任意步骤         |
| 🌈 **变体系统**        | 自动递归生成所有材质+差分的组合，每个变体独立打包            |
| 📝 **自定义 VMT 模板** | 支持 `{{base}}`、`{{light}}`、`{{bump}}`、`{{env}}` 占位符   |
| 📊 **进度反馈**        | 实时进度条和完整日志记录                                     |

---

## 🛠️ 技术栈

<div align="center">

| 技术          | 用途                            |
| ------------- | ------------------------------- |
| **C++17**     | 核心语言                        |
| **Qt 6.10**   | GUI 框架、信号槽、进程管理      |
| **FreeImage** | TGA 图像处理（Alpha 通道）      |
| **QProcess**  | 调用外部工具（MareTF、vpk.exe） |
| **QSettings** | 用户配置持久化                  |

</div>

---

## 📁 项目结构

L4D2MaterialsHelper/
├── 🖥️ mainwindow.cpp/h # 主窗口
├── 🔧 componentsetting.cpp/h # 组件配置
├── 🎨 materialsetting.cpp/h # 材质配置
├── 🔀 variantconfigdialog.cpp/h # 变体配置
├── 📦 processors/ # 核心处理模块
│ ├── texturepreprocessor.cpp/h
│ ├── vtfgenerator.cpp/h
│ ├── vmtgenerator.cpp/h
│ └── vpkgenerator.cpp/h
├── 🛠️ utils/fileutils.h # 工具类
└── 📋 datastructures.h # 数据结构

---

## 🚀 使用步骤

1. **配置路径**  
   设置源文件夹、输出文件夹、maretf.exe、vpk.exe 路径

2. **创建组件**  
   点击“增加组件”，输入组件名（如“鞋子”），添加材质（中文名+英文名）

3. **配置材质**  
   选择组件和材质，配置贴图路径和渲染参数

4. **配置变体**  
   为每个组件创建变体，选择每个材质使用的差分

5. **执行**  
   点击“执行”，查看日志和进度条

---

## 📦 依赖工具

- [MareTF](https://github.com/craftablescience/MareTF) - VTF 转换工具（已放置）
- [VPK Tool](https://developer.valvesoftware.com/wiki/VPK) - Valve 打包工具（已放置）
- [FreeImage](https://freeimage.sourceforge.io/) - 图像处理库（需自行下载）

---

## 💻 环境要求

- Windows 10 / 11
- Qt 6.10+（如需编译源码）
- Visual Studio 2022 或 MinGW

---

## 🔨 构建与运行

```bash
# 克隆仓库
git clone https://github.com/Glyer1/L4D2MaterialsHelper.git

# 用 Qt Creator 打开 .pro 文件
# 编译运行
```

## 📄 许可证

本项目采用 **MIT License** 开源协议。

---

## 📧 联系我

- GitHub: [@Glyer1](https://github.com/Glyer1)
- 邮箱: 1848200838@qq.com

---

<div align="center">
⭐ 如果这个项目对你有帮助，欢迎点个 Star ⭐
</div>
