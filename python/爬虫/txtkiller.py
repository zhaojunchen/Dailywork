# 目标网站 http://m.cuiweijux.com
# 功能 爬取小说的网页
# 使用方法 在main的注释里面
import threading
import time
import requests
from bs4 import BeautifulSoup
import queue as Queue

# 获取小说方法一 单线程 速度较慢 原理 小说得下一页包含跳转链接
# 输入小说的第一章节（或者其他的章节）的url 即可获的（后续章节）小说的txt文本
# 输入第一章节的url名称就可以得到后续的所有txt
# get_txt_by_txtindexhtml("http://m.cuiweijux.com/files/article/html/1/1045/609424.html")
def get_txt_by_txtindexhtml(url):
    domain = "http://m.cuiweijux.com"
    try:
        with open("./" + url.split('/')[-2] + ".txt", 'w', encoding='utf-8') as f:
            while True:
                soup = requests.get(url)
                soup = BeautifulSoup(soup.content.decode("gbk"), "lxml")
                title = soup.find(id='nr_title').text.strip()
                f.write(title + '\n\n')
                passage = soup.find(id='nr1')
                passage = passage.text.split()
                passage = "\n".join(passage)
                f.write(passage + "\n\n")
                url = domain + soup.find(id="pt_next")['href']
                print(url)
                if url == "http://m.cuiweijux.com/files/article/html/1/1045/index.html":
                    break
    except Exception as e:
        print(url + " error")
        print(e)



# 获取方法二 显示用单线程获取小说得展示目录
# 将所有的页面链接（添加章节信息）放入queue里面 采用多线程加速爬取
# 将爬取的内容按照章节提前分配的章节顺序 写入文件里面（本目录下）
# 可以的优化 由于首页可以得到总页数 故而可以使用多线程爬取目录 但是需要注意页面顺序

class myThread(threading.Thread):

    def __init__(self, name, q):
        threading.Thread.__init__(self)
        self.name = name
        self.q = q

    def run(self):
        print("Starting " + self.name)
        while True:
            try:
                crawler(self.name, self.q)
            except Exception as e:
                print(e)
                break
        print("Exiting " + self.name)

# 爬取文本内容
def crawler(threadName, q):
    # 在队列塞满了以后，在阻塞了timeout时间后raise一个queue.full的异常
    # 去除url参数
    response = q.get(timeout=2).split('-')
    url = domain + response[1]
    index = response[0]
    print("index: " + index+ "url: " + url)

    try:
        soup = requests.get(url)
        soup = BeautifulSoup(soup.content.decode("gbk"), "lxml")
        title = soup.find(id='nr_title').text.strip()
        passage = soup.find(id='nr1')
        passage = passage.text.split()
        passage = "\n".join(passage)
        passage = title + "\n\n" + passage + '\n\n'
        dict[index] = passage

    except Exception as e:
        print(e)


# 获取目录的url内容到这个队列


# url = "http://m.cuiweijux.com/mulu/1/1045/"
# q输入队列
def get_url_into_queue(url, q):
    i = 1
    j = 0
    url = url[0:-1] + '_'

    while (True):
        try:
            soup = requests.get(url + str(i) + '/')
            soup = BeautifulSoup(soup.content.decode("gbk"), "lxml")
            soup = soup.find(attrs={'class': "chapter"})
            soup = soup.find_all('a')
            if len(soup) == 0:
                break
            for each in soup:
                print(str(j) + '-' + each['href'])
                q.put(str(j) + '-' + each['href'])
                j += 1
            i = i + 1
            # with open("./zjc1.txt", 'a+', encoding="utf-8") as f:
            #     for each in soup:
            #         print(each.text)
            #         f.write(each.text + '\n')

            # print(soup[0].text)
        except Exception as e:
            print(e)
            break

if __name__ == '__main__':
    # 采用方法二 爬取小说
    start = time.time()
    domain = 'http://m.cuiweijux.com'
    dict = {}
    workQueue = Queue.Queue()
    # 添加目标网页  展开首页的完整目录 copy到url
    url = "http://m.cuiweijux.com/mulu/0/944/"
    soup = requests.get(url)
    soup = BeautifulSoup(soup.content.decode("gbk"), "lxml")
    txt_name = soup.find(attrs={'class': 'read'}).find('h3').text.strip()
    get_url_into_queue(url, workQueue)

    threads = []
    for i in range(10):
        thread = myThread("threading-" + str(i), workQueue)
        thread.start()
        threads.append(thread)

    for each in threads:
        each.join()

    with open('./' + txt_name + ".txt", 'w', encoding='utf-8') as f:
        for i in range(len(dict)):
            f.write(dict[str(i)])

    end = time.time()

    print(str(end - start) + "s")
    print("Exiting...")
