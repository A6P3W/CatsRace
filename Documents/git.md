# git rule
## 同期
### 開発開始時
```
git pull
git submodule update --init --recursive
```
### Engine変更後
1.  Engine内
```
git checkout
git add
git commit
git push
```
2. 上位内
```
git add
git commit
git push
```

## ブランチ名
feature/