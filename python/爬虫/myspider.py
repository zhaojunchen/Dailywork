import requests
import os
import pymysql
from bs4 import BeautifulSoup

# http://3332v.com/
domain = "http://3332v.com/htm/Video4/"
path = "C:\\pic"

db = pymysql.connect(host='127.0.0.1', user='root', password='liujiao', db='yellow', port=3306, charset='utf8')
cursor = db.cursor()
if not db or not cursor:
    print("error")
    exit(-111)

# cursor.execute("DROP TABLE IF EXISTS movies;")
cursor.execute("USE yellow")

sql = """CREATE TABLE IF NOT EXISTS movies (
		 movieid INT UNSIGNED AUTO_INCREMENT,
         moviename VARCHAR(100),
         movielink  VARCHAR(200),
         movieimage MEDIUMBLOB,
         PRIMARY KEY (movieid));"""


# try:
#     # 执行sql语句
#     cursor.execute(sql)
#     # 提交到数据库执行
#     db.commit()
# except:
#     # Rollback in case there is any error
#     db.rollback()
#     print("数据库创建失败")


cursor.execute(sql)

sql = "INSERT INTO movies (moviename, movielink, movieimage) VALUES (%s, %s, %s);"

cookies = {
    '__cfduid': 'd6d436ea9fe3d93c1797cc330731e05431568007644',
    'UM_distinctid': '16d1489ef09c3-0fe586a117f4f8-5373e62-19fa51-16d1489ef0a23',
    '_ga': 'GA1.2.1680426548.1568007647',
    '_gid': 'GA1.2.1488742013.1568007647',
    'Hm_lvt_a9058c4d8240a8d5d9d68d3d116c7697': '1568014355',
    'Hm_lvt_ae79515eb4cc983e50647ac494bae4d6': '1568014356',
    'Hm_lpvt_ae79515eb4cc983e50647ac494bae4d6': '1568014356',
    'CNZZDATA1278008611': '1196233660-1568009704-%7C1568015105',
    'Hm_lpvt_a9058c4d8240a8d5d9d68d3d116c7697': '1568015967',
}

headers = {
    'Connection': 'keep-alive',
    'Cache-Control': 'max-age=0',
    'Upgrade-Insecure-Requests': '1',
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/76.0.3809.132 Safari/537.36',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3',
    'Referer': 'http://3332v.com/htm/Video4/2.htm',
    'Accept-Encoding': 'gzip, deflate',
    'Accept-Language': 'zh-CN,zh;q=0.9,en;q=0.8',
}

def mkdir_storage(path):
    if not os.path.exists(path):
        print("waiting mkdir " + path + "...")
        os.mkdir(path)
        print(path + "dir create ok")
    else:
        print(path + "exists!")
        # change file path
        # os.chdir(path)


# 函数将图片的信息 保存在本地磁盘
# 传入图片的url 传入的图片名字参数
def save_img(url, name):  # 保存图片
    img = requests.get(url, headers=headers)
    name = name + ".png"
    print(img.content)
    c = img.content
    print(type(img.content))
    with open(name, "ab") as f:
        f.write(img.content)
        print(name, '文件保存成功！')


response = requests.get('http://3332v.com/htm/Video4/1.htm', headers=headers, cookies=cookies, verify=False)

soup = BeautifulSoup(response.content.decode("utf-8"), "lxml")
print(soup)
# findAll 查找所有的属性节点  find之查找第一个节点
div_target = soup.find(name="div", attrs={"class": "box dy_list"})
# print(type(div_target))
# print(div_target)
all_target = div_target.findAll('a')
# print(type(all_target))
# print(all_target)

print("打印出来结果\n切换目录")
mkdir_storage(path)
os.chdir(path)
print(os.getcwd())
if not os.getcwd() == path:
    print("error")
    exit(-1111)
# 重要的算法
# for i in all_target:
#     movie_link = "http://3332v.com" + i.get("href")
#     print(movie_link)
#     # https://bbs.csdn.net/topics/391011314?list=lz 如何获取a标签的href和title
#     # 获取标签里面的纯文本
#     movie_name = i.h3.text.strip()
#     print(movie_name)
#     image_link = i.img.get("src")
#     print(image_link)
#     print("store... " + movie_name)
#     # save_img(image_link, movie_name)
#     img = requests.get(image_link, headers=headers)
#     # image_name = name + ".png"
#     if not cursor:
#         print("fuck you!!! cursou is null")
#     print(sql)
#     cursor.execute(sql, args=(movie_name, movie_link, pymysql.Binary(img.content)))

# print(div_target)

cursor.execute("SELECT * FROM movies LIMIT 2;")

cs = cursor.fetchone()[3]
mkdir_storage("C:\\zjc")
os.chdir("C:\\zjc")
with open("myzjc.png","ab") as f:
    f.write(cs)
    print("文件传输完成")


def mkdir_storage(path):
    if not os._exists(path):
        print("waiting mkdir " + path + "...")
        os.mkdir(path)
        print(path + "dir create ok")
    else:
        print(path + "exists!")
        # change file path
        # os.chdir(path)


def save_img(url, name):  # 保存图片
    img = self.request(url)
    name = name + ".png"
    os.chdir(path)
    with open(name, "ab") as f:
        f.write(img.content)
        print(name, '文件保存成功！')

# print(response.content)
# print(response.content.decode("utf-8"))
# print(response.text)
# soup = BeautifulSoup(response.content,"lxml")
# print(soup)
