# linux_repo

### 리눅스 커널 자료구조를 이용한 성능 개선 프로젝트
- Red-Black Tree 자료구조의 critical section을 tree 전체에서 leaf node로 축소해 parallel하게 node를 삽입함으로써 성능을 개선했습니다.
- parallel 하게 tree node를 삽입하는 것은 multi-threading으로 인해 기존 자료구조보다 약 2.6의 삽입 성능을 개선할 수 있었습니다.
