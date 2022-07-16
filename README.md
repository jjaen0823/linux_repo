# linux_repo

## [ 리눅스 커널 자료구조를 이용한 성능 개선 프로젝트 ]
- Red-Black Tree 자료구조의 critical section을 tree 전체에서 leaf node로 축소해 parallel하게 node를 삽입함으로써 성능을 개선했습니다.
- parallel 하게 tree node를 삽입하는 것은 multi-threading으로 인해 기존 자료구조보다 약 2.6의 삽입 성능을 개선할 수 있었습니다.

### Background of Implementation
---
- Red-Black Tree는 recoloring 작업 때문에 node 삽입 시, tree 전체가 critical section이다.
- multi-threading을 적용한다해도 삽입의 경우는 하나의 thread만 tree에 접근할 수 있어 locking overhead로 인해 오히려 삽입 시간이 증가하게 된다.
<img width="400" alt="image" src="https://user-images.githubusercontent.com/75469281/179346570-7b1a0290-3b06-418b-a4bf-60d537fc594a.png">

### Key Idea
#### : linked list in all leaf nodes of RB tree
---
- RE tree의 모든 leaf node에 linked list를 연결함으로써, critical section을 leaf node로 축소할 수 있다.
<img width="800" alt="image" src="https://user-images.githubusercontent.com/75469281/179346805-5b4010e8-bb4e-48e3-8b50-ac64c41fcf68.png">


