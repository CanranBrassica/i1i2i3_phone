# i1i2i3 phone

eeicのi3実験の成果物

## 準備

make, cmake, boost(1.66以上?)をインストール

```
$ sudo apt update
$ sudo apt install build-essential cmake
```

boostのインストールはhttps://boostjp.github.io/howtobuild.htmlなどを参考

## ビルド

このディレクトリで
```
$ mkdir build && cd build
$ cmake ..
$ make -j4
```

## 実行

まず，サーバーを立ち上げる

```$xslt
$ ./server <port>
```

次にクライアントを立ち上げる．

```$xslt
$ ./client <server ip address> <server port>
```

クライアントは立ち上げると，サーバーとTCP/IPによるコネクションを確立する．
コネクションに成功すると，サーバーから固有のユーザーIDが割り振られる．

この通信プログラムはroomを備えており，同じroomにjoinしているクライアント同士でのみ通信が可能である．
まず，roomを作るには`/create_room <room_id>`コマンドを用いる．
すでに使われているroom_idを指定した場合は失敗する．
roomにjoinするには`/join_room <room_id>`とする．
また，`/room_list`とすると，現在作られているroomの一覧とそこにjoinしているクライアントの一覧が見れる．

roomにjoinした状態で，`/`から始まらない文字列を入力すると，これが全クライアントの標準入力に拡散される．
これにより，テキストによるチャット機能が使える．

roomを作ると，サーバーによって固有のUDPマルチキャストのグループアドレスとポート番号が割り振られる．
`/start_multicast`とすると，この情報をサーバーから取得し，UDPマルチキャスト通信に参加できるようになる．
UDPマルチキャスト通信を用いることで，サーバーを介さずにroomに参加しているクライアント同士で多対多の通信が可能となる．

このUDPマルチキャストを用いて音声通話を行える．
`/start_multicast`をした状態で，`/join_call`とすると音声通話に参加できる．
また，他のグループ参加者が`/join_call`するとサーバーから参加通知がされる．
音声通話をしながらでも，テキストチャットは継続する．

通話に参加している時に`/leave_call`とすると，通話を切断できる．
同様に`/leave_multicast`によりマルチキャストグループからの離脱，
`/leave_room`により，roomからの離脱ができる．

