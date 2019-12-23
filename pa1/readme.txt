1. 學號：
2. 姓名：
3. 使用之程式語言：< C++ >
4. 使用之編譯器：< GNU g++ >
5. 檔案壓縮方式: <tar cvfz A15.tar.gz A15>
6. 各檔案說明：
	 A60/Source/Explode : 純文字主程式
	 A60/Source/Verify  : 純文字驗證程式
	 A60/Source/Visulize : 視窗繪圖程式（相容於WindowsXP、Windows2000等）
	 A60/Report.doc ： 程式報告

7. 編譯方式說明：        	
   主程式：A60/Source/Explode
	 請在主程式的目錄下，鍵入make指令，即可完成編譯，
	 ( Makefile原先下的 Optimize 指令為 -O2
	 若是要改用 -O 請使用同資料匣下的 Makefile2 )
	 在主程式的目錄下會產生一個名為 a.out 的執行檔
	 如果要重新編譯，請先執行 make clean 再執行一次 make
	
   驗證程式：同"主程式"

   視窗繪圖程式：對於視窗介面繪圖驗證程式在A60/Source/Visualize/Release中已附
	 加Visualize.exe的執行檔。在視窗平台上點擊兩下執行檔，即可
	 開啟繪圖程式。若要重新編譯，請用Visual Studio.NET開啟Source/Visualize/
	 Visulize.sln檔案。確定Active Configuration為"Release"之後，
	 選擇Build→Build Solution，進行編譯。在Release目錄下會產生執行檔。

           . . . . . .  
8. 執行、使用方式說明：
   主程式：
   編譯完成後，在檔案目錄下會產生一個 a.out 的執行檔
   執行檔的命令格式為：
   ./a.out <input file name> <top cell name>
   例如：要對 V07Q.dat 的 cell V07Q 作explode
   則在命令提示下鍵入
   ./a.out V07Q.dat V07Q

   驗證程式：執行檔名為verify，命令格式與主程式相同
           . . . . . .         
9. 執行結果說明（說明執行結果的觀看方法，及解釋各項數據等）：
   主程式：
	 主程式執行時會在 console 輸出目前執行的步驟、頂層元件名稱、
	 最大階層數、實體佈局幾何圖形數、與各步驟的執行時間。
	 輸出檔為在本目錄下的 result.dat ， 另外會輸出未作 explode
	 而頂層元件有參考到的 physical layout 到 original.dat
	 例如：執行 ./a.out V07Q.dat V07Q 後，console 會輸出
	
 	 Program initializing
	 Top cell name: V07Q						-- 頂層元件名稱
	 Max hierarchical level: 12					-- 最大階層數
	 Real rects count = 398863					-- 幾何圖形數
	 Start of overlapping checking
	 End of overlapping checking
	 Start of exploding
	 Finishing program and writing result
	
	
	 Program initialization spends 0.420000 seconds			-- 各項步驟計時
	 Overlapping graph construction spends 65.780000 secnods
	 Removing of hierarchical overlapping spends 0.700000 secnods
	 Finishing program and writing results spends 2.600000 seconds

	 Total program time = 69.500000 seconds				-- 程式總執行時間



       

