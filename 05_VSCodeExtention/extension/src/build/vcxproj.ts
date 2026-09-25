import * as crypto from 'crypto';
import * as fs from 'fs';
import * as path from 'path';

/**
 * MSBuild / Visual Studio 用のプロジェクトファイル(DESIGN.md 6 章)。
 * 拡張機能が作って管理し、生徒は触らない。ソースは src\**\*.cpp のワイルドカードで取り込む。
 *   <名前>.vcxproj … ビルドの設定(以前の cl の指定と同じ)
 *   <名前>.sln     … Visual Studio でダブルクリックして開くため
 *   dxlib.props    … DxLib SDK の場所(PC ごとに違うので分ける。.gitignore・テンプレートに入れない)
 */

/** この拡張が書いた .vcxproj の印。印の無い .vcxproj(Visual Studio で作ったもの)は書き換えない。 */
const MARKER = '<DxLibDevEnv>1</DxLibDevEnv>';

/** この拡張が作った .vcxproj の中身か(印があるか)。 */
export function isOwnVcxprojText(text: string): boolean {
	return text.includes(MARKER);
}
const VC_PROJECT_TYPE = '8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942';

export interface ProjectFilePaths {
	vcxproj: string;
	sln: string;
	props: string;
}

export function projectFilePaths(projectDir: string, name: string): ProjectFilePaths {
	return {
		vcxproj: path.join(projectDir, `${name}.vcxproj`),
		sln: path.join(projectDir, `${name}.sln`),
		props: path.join(projectDir, 'dxlib.props'),
	};
}

function newGuid(): string {
	return crypto.randomUUID().toUpperCase();
}

function xmlEscape(s: string): string {
	return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

/** 設定の C++ 規格(c++17 / c++20 / c++latest)を .vcxproj の書き方(stdcpp20 など)にする。 */
function languageStandard(cppStandard: string): string {
	const m = /^c\+\+(\d+|latest)$/i.exec(cppStandard.trim());
	return m ? `stdcpp${m[1].toLowerCase()}` : 'stdcpp20';
}

export function vcxprojText(name: string, guid: string, cppStandard: string): string {
	const n = xmlEscape(name);
	return `<?xml version="1.0" encoding="utf-8"?>
<!-- DxLib 開発環境(VSCode 拡張機能)が作ったファイルです。書き換えないでください(拡張機能が作り直します)。 -->
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    ${MARKER}
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{${guid}}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
    <RootNamespace>${n}</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
  <PropertyGroup Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <PlatformToolset>$(DefaultPlatformToolset)</PlatformToolset>
    <CharacterSet>MultiByte</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)'=='Debug'" Label="Configuration">
    <UseDebugLibraries>true</UseDebugLibraries>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)'=='Release'" Label="Configuration">
    <UseDebugLibraries>false</UseDebugLibraries>
    <WholeProgramOptimization>false</WholeProgramOptimization>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <Import Project="dxlib.props" Condition="Exists('dxlib.props')" />
  <PropertyGroup>
    <OutDir>$(ProjectDir)build\\$(Configuration)\\</OutDir>
    <IntDir>$(ProjectDir)build\\$(Configuration)\\obj\\</IntDir>
    <TargetName>${n}</TargetName>
    <IgnoreWarnIntDirInTempDetected>true</IgnoreWarnIntDirInTempDetected>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <DisableSpecificWarnings>4819;%(DisableSpecificWarnings)</DisableSpecificWarnings>
      <LanguageStandard>${languageStandard(cppStandard)}</LanguageStandard>
      <MultiProcessorCompilation>true</MultiProcessorCompilation>
      <AdditionalOptions>/source-charset:.932 /execution-charset:.932 %(AdditionalOptions)</AdditionalOptions>
      <AdditionalIncludeDirectories>$(DxLibDir);$(ProjectDir)src;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>_WINDOWS;WIN32;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <AdditionalLibraryDirectories>$(DxLibDir);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)'=='Debug'">
    <ClCompile>
      <Optimization>Disabled</Optimization>
      <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
      <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
    <Link>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)'=='Release'">
    <ClCompile>
      <Optimization>MaxSpeed</Optimization>
      <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="src\\**\\*.cpp" />
    <ClInclude Include="src\\**\\*.h;src\\**\\*.hpp" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
</Project>
`;
}

export function slnText(name: string, projectGuid: string, solutionGuid: string): string {
	return `
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.0.31903.59
MinimumVisualStudioVersion = 10.0.40219.1
Project("{${VC_PROJECT_TYPE}}") = "${name}", "${name}.vcxproj", "{${projectGuid}}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Release|x64 = Release|x64
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{${projectGuid}}.Debug|x64.ActiveCfg = Debug|x64
		{${projectGuid}}.Debug|x64.Build.0 = Debug|x64
		{${projectGuid}}.Release|x64.ActiveCfg = Release|x64
		{${projectGuid}}.Release|x64.Build.0 = Release|x64
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {${solutionGuid}}
	EndGlobalSection
EndGlobal
`;
}

export function propsText(sdkPath: string): string {
	return `<?xml version="1.0" encoding="utf-8"?>
<!-- DxLib SDK の場所(この PC 用)。DxLib 開発環境(VSCode 拡張機能)が書き直します。 -->
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <DxLibDir>${xmlEscape(sdkPath)}</DxLibDir>
  </PropertyGroup>
</Project>
`;
}

/** Visual Studio と同じく BOM 付き UTF-8・CRLF で書く。中身が同じなら書かない(書いたら true)。 */
function writeIfChanged(file: string, text: string): boolean {
	const content = '﻿' + text.replace(/\r?\n/g, '\r\n');
	try {
		if (fs.readFileSync(file, 'utf8') === content) {
			return false;
		}
	} catch {
		// 無ければ書く
	}
	fs.writeFileSync(file, content, 'utf8');
	return true;
}

function readGuid(text: string, re: RegExp): string | undefined {
	const m = re.exec(text);
	return m ? m[1].toUpperCase() : undefined;
}

export interface EnsureResult {
	/** 作った・書き直したファイル(通知や検証に使う)。 */
	written: string[];
	/** 同じ名前の .vcxproj があるが、この拡張が作ったものではないので触らなかった。 */
	foreignVcxproj?: string;
}

/**
 * .vcxproj・.sln・dxlib.props を今の設定に合わせる(無ければ作る)。
 * .vcxproj と .sln の GUID は、書き直しても前のものを使い続ける(Visual Studio の設定が外れないように)。
 * sdkPath が空なら dxlib.props は書かない(SDK が決まったときに書く)。
 */
export function ensureProjectFiles(projectDir: string, name: string, sdkPath: string, cppStandard: string): EnsureResult {
	const p = projectFilePaths(projectDir, name);
	const written: string[] = [];
	let oldVcx = '';
	try {
		oldVcx = fs.readFileSync(p.vcxproj, 'utf8');
	} catch {
		oldVcx = '';
	}
	if (oldVcx && !oldVcx.includes(MARKER)) {
		return { written, foreignVcxproj: p.vcxproj };
	}
	const projectGuid = readGuid(oldVcx, /<ProjectGuid>\{([0-9A-Fa-f-]+)\}<\/ProjectGuid>/) ?? newGuid();
	if (writeIfChanged(p.vcxproj, vcxprojText(name, projectGuid, cppStandard))) {
		written.push(p.vcxproj);
	}
	let oldSln = '';
	try {
		oldSln = fs.readFileSync(p.sln, 'utf8');
	} catch {
		oldSln = '';
	}
	const solutionGuid = readGuid(oldSln, /SolutionGuid = \{([0-9A-Fa-f-]+)\}/) ?? newGuid();
	if (writeIfChanged(p.sln, slnText(name, projectGuid, solutionGuid))) {
		written.push(p.sln);
	}
	if (sdkPath && writeIfChanged(p.props, propsText(sdkPath))) {
		written.push(p.props);
	}
	return { written };
}
