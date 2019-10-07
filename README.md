# mtlpp_tutorial
mtlppというMetalのC++ラッパーのチュートリアル。

## tutorial_00
三角形を表示するためのサンプルプログラム。

・ライブラリの設定  
Xcodeの[Build Phases]の Link Binary With Librariesに以下のフレームワークを追加する。  
  Cocoa.framework  
  Metal.framework  
  MetalKit.framework  

・mtlpp.mmのコンパイルオプションの設定  
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
・実行時のワーキングディレクトリの設定
