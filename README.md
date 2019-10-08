# mtlpp_tutorial
mtlppというMetalのC++ラッパーを使用したチュートリアル。

## tutorial_00
三角形を表示するためのサンプルプログラム。

・ライブラリの設定  
Xcodeの[Build Phases]の Link Binary With Librariesに以下のフレームワークを追加する。  
  Cocoa.framework  
  Metal.framework  
  MetalKit.framework  

・Objective-Cのソースファイルのコンパイルオプションの設定  
mtlppの ```mtlpp.hpp``` と ```mtlpp.mm``` はそのままプロジェクトに追加する。  
同様にmtlppのサンプルプログラムにある ```window.hpp``` と ```window.mm``` も持ってきてそのままプロジェクトに追加する。  
mtlppはARCには非対応となっているためそのままではビルドエラーになってしまう。  
Xcodeの[Build Phases]の Compile Sources のCompiler Flagsに以下のオプションを追加する。  
  -fno-objc-arc  

## tutorial_01
tutorial_00のシェーダーの記述を *.metal に置き換えたサンプルプログラム。  
*.metalをXcodeのプロジェクトに追加すると一緒にビルドされ NewDefaultLibrary でアクセスできるようになる。  

Appleのドキュメントのリンク  

## tutorial_02
tuorial_01のシェーダーファイルをリソースとして読み込んで使用するサンプルプログラム。 
MSL(Metal Sharding Language)の *.metal ファイルをコマンドラインでビルドして *.metallib を作成して、そのファイルを読み込んでシェーダーを使用するサンプルプログラム。  

・metalファイル  
  basic.metal  
  basic2.metal  

・metallibのビルド方法  
  ターミナルなどから以下のコマンドを実行する。  
  
  *.metalをコンパイルして中間フォーマットの *.air を作成。  
    $ xcrun -sdk macosx metal basic.metal -c -o basic.air

  *.metallibをビルドする.  
    複数の *.air をまとめてビルドすることも可能。  
    その際、関数名が重複してしまうとビルドエラーになってしまうので注意が必要。  
    $ xcrun -sdk macosx metallib basic.air basic2.air -o basic.metallib

・ワーキングディレクトリの設定  
  プロジェクトファイル直下にあるリソースを読み込むためにワーキングディレクトリの設定を行う。  
  メインメニューの [Product] -> [Scheme] -> [Edit Scheme] の Run の Optionタブの Working Directory の Use custom working directory: にチェックを入れる。  
  項目に $(SRCROOT) と入力。  

## tutorial_03
テクスチャを適用した矩形の描画のサンプルプログラム。  
テクスチャの読み込みがmtlppでは用意されていない模様...  
MetalKitを使用してテクスチャの読み込みを行う。  

