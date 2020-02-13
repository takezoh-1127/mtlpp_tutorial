# mtlpp_tutorial
mtlppというMetalのC++ラッパーを使用したチュートリアル。

[mtlpp C++ Metal wrapper](https://github.com/naleksiev/mtlpp)


macOS : 10.15.1  
Xcode : 11.3  

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



## tutorial_02
tutorial_01のシェーダーファイルをリソースとして読み込んで使用するサンプルプログラム。  
MSL(Metal Sharding Language)の *.metal ファイルをコマンドラインでビルドして *.metallib を作成して、そのファイルを読み込んでシェーダーを使用するサンプルプログラム。  

[Building a Library with Metal's Command-Line Tools](https://developer.apple.com/documentation/metal/libraries/building_a_library_with_metal_s_command-line_tools)

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
mtlppにはテクスチャを読み込むためのAPIが用意されていない模様...  
MetalKitのMTKTextureLoaderを使用してと外部ファイルのテクスチャを読み込めるようにする。  
mtlppに倣ってMTKTextureLoaderのらラッパークラスを以下のファイルで定義。  
  mtlpp/textureloader.hpp  
  mtlpp/textureloader.mm  

<img width="640" alt="tutorial_03" src="https://user-images.githubusercontent.com/27395939/74458595-4ae4eb00-4ecd-11ea-86fc-aef535919f5d.png">

## tutorial_04
tutorial_03をインデックスバッファを使用した描画に変更したサンプルプログラム。

## tutorial_05
立方体の描画のサンプルプログラム。  
立方体はLambertで描画。  
ワールドスペースで計算。

## tutorial_06
複数の立方体の描画のサンプルプログラム。  
立方体はPhongシェーダーで描画。  
ワールドスペースで計算。

## tutorial_07
tutorial_05のlambertをローカルスペースの計算に書き換え。


## tutorial_08
tutorial_06のPhongシェーディングをローカルスペースの計算に書き換え。

<img width="640" alt="tutorial_08" src="https://user-images.githubusercontent.com/27395939/74458905-bcbd3480-4ecd-11ea-959e-be93afa47655.png">

## tutorial_09
objファイルを読み込んで表示するサンプルプログラム。

<img width="640" alt="tutorial_09" src="https://user-images.githubusercontent.com/27395939/74457771-13297380-4ecc-11ea-9e5f-9d1e0f77c4a5.png">


## 予定
今後作成するチュートリアル、サンプルの予定。  
・ノーマルマップ  
・objファイルの読み込み、表示  
・glTFファイルの読み込み、表示
・FBXファイルの読み込み、表示
・PBR  
・デファードレンダリング  
・ポストエフェクト  




