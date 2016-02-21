現在このビルド方法は執筆中です。ただしくビルドできない場合があります。
その時はIssueでもなんでも飛ばしてくれると補足を追加します。

# はじめに
このプログラムは2016/02/13-14に開催されたハッカソンイベント
HackDay2016において開発されたものです。  
詳細は以下を参照ください  
[Youtube:HackID 88 立体逓信団](https://youtu.be/lsujyuPHnxE?t=3h49m7s)

# ビルド方法

## CMakeの利用
コマンドラインでCMakeを実行する場合、cmakeコマンドではなくccmakeコマンドの
利用を推奨します。  
ccmakeコマンドの利用方法はcmakeと同じですが、コマンドライン上でありながら
GUIと同じ操作感でCMake変数の確認・編集を行うことができます。
CMakeを自前でビルドした場合、ccmakeコマンドがない場合があります。
その場合はncurseパッケージをインストールし、再ビルドすることでccmakeが利用可能です。

## OpenCVのビルド
本プログラムではOpenCVは3.1以上が必須となっています。
またOpenCVのメインリポジトリには含まれていない機能を利用しています。
そのためOpenCVの再ビルドが必要な場合があります。

下記のリポジトリからソースコードをダウンロードします。
opencvは3.1を利用します
[opencv on github](https://github.com/Itseez/opencv)
[opencv_contrib on github](https://github.com/Itseez/opencv_contrib)

また、動画の読み込みを前提としているためffmpegを利用します。
Windowsは自動でビルド時にダウンロードされますが、Linuxではコーデックのビルド
から行わなければならない場合があります。
ほとんどの場合、パッケージが用意されているのでそちらの利用を推奨します。
[ffmpeg](https://www.ffmpeg.org/download.html)

CMake実行後、OPENCV_EXTRA_MODULE_PATH変数にopencv_contrib/moduleディレクトリ
を追加し、再度CMakeを実行します。GUI、またはccmakeの場合はconfigureを行います。  
すると初回実行時にはなかった変数が多数追加されます。
追加された変数の中にopencv_ximgprogがあることを確認してください。
本プログラムではこの変数によってビルドされるライブラリを利用しています。

### Linux
作成されたMakefileをビルド、インストールしてください。  
`make`  
`sudo make install`

ビルド完了後、CMAKE_PREFIX_PATHを環境変数に加えます。
CentOSの場合、値として/usr/local/share/OpenCVを入力します。
設定するパス以下にはOpenCVConfig.cmakeとOpenCVConfig-version.cmakeが必要です。

### Windows
VisualStudioの場合、Debug、Release双方でALL_BUILDとINSTALLの二つを順にビルドします。
ソリューションの右クリックから行えるバッチビルドを使うと便利です。  
ビルド完了後、CMAKE_PREFIX_PATHを環境変数に加え、ビルドパスにあるinstallディレクトリを値として入力します。
installディレクトリにはOpenCVConfig.cmake、OpenCVConfig-version.cmakeの二つのファイルがあります。

## 本プログラムのビルド
CMakeを実行し、Makefileまたはソリューションファイルを作成します。
このとき、OpenCVのビルドで行ったCMAKE_PREFIX_PATHが正常に設定されているとCMakeが自動的に
OpenCVの設定を読み込みます。

### Linux
!!!!注意!!!!  
現在の本プログラムはVisual Studio上で開発・動作確認を行っています。
そのため、そもそもビルドできない場合があります。ご了承ください。  
今後Linux上でも動作確認を行う予定です。

作成されたMakefileを使いビルドします。インストールの設定はありません。  
`make`

### Windows
ソリューションファイルをVisual Studioで開き、ビルドします。
デバッガーによる実行を行う場合はスタートアッププロジェクトをmainに変更してから実行してください。
ただし、コマンドライン引数が必要なため実行後、ヘルプを表示して終了してしまいます。  
実行する場合はReleaseまたはDebugディレクトリ以下に生成されるmain.exeを利用してください。

# 使い方
簡単な使い方  
`main <video file>`

より詳しい使い方は以下を参照  
`main --help`  