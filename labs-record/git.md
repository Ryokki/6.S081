```shell
git clone git://g.csail.mit.edu/xv6-labs-2020	#从mit的库中clone进本地(origin,连接到mit)
git checkout util  # 选择mit远程库的第一个
git remote add my https://github.com/A17764530215/6.S081	#创建my,连接到我的github

git add .
git commit -m "lab1"
git push -u my util		#当前分支下的commit的数据传到my(我的github)的util分支

```

