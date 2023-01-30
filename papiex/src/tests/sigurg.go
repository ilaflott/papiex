func main() {
   var total int
   var wg sync.WaitGroup

   for i := 0; i < 20; i++ {
      wg.Add(1)
      go func() {
         for j := 0; j < 1e6; j++ {
            total ++
         }
         wg.Done()
      }()
   }

   wg.Wait()
}
