        // Base URL for the API
        const API_BASE_URL = 'http://localhost:8080';
        
        // DOM elements
        const memoryVisualization = document.getElementById('memoryVisualization');
        const fragmentationStats = document.getElementById('fragmentationStats');
        const processList = document.getElementById('processList');
        const notification = document.getElementById('notification');
        const totalMemorySizeDisplay = document.getElementById('totalMemorySize');
        const toggle_text = document.querySelector('.toggle-text');
        toggle_text.textContent = 'Dark Mode';
    // Wait for DOM to fully load
    document.addEventListener('DOMContentLoaded', function () {
        const toggle = document.querySelector('.toggle');
        const body = document.body;

        toggle.addEventListener('click', () => {

            body.classList.toggle('light-mode');
            toggle.classList.toggle('light');
            if(body.classList.contains('light-mode')){
                toggle_text.textContent='Light Mode';
            }else{
                toggle_text.textContent='Dark Mode';
            }
        });
    });

        // Tab functionality
        document.querySelectorAll('.tab-button').forEach(button => {
            button.addEventListener('click', () => {
                const tabId = button.getAttribute('data-tab');
                
                // Update active tab button
                document.querySelectorAll('.tab-button').forEach(btn => {
                    btn.classList.remove('active');
                });
                button.classList.add('active');
                
                // Update active tab content
                document.querySelectorAll('.tab-content').forEach(content => {
                    content.classList.remove('active');
                });
                document.getElementById(tabId).classList.add('active');
            });
        });
        
        // Show notification
        function showNotification(message, isError = false) {
            notification.textContent = message;
            notification.className = 'notification';
            if (isError) {
                notification.classList.add('error');
            }
            notification.style.display = 'block';
            
            setTimeout(() => {
                notification.style.display = 'none';
            }, 3000);
        }
        
        // Fetch data from API
        async function fetchData(endpoint) {
            try {
                const response = await fetch(`${API_BASE_URL}${endpoint}`);
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return await response.json();
            } catch (error) {
                console.error('Error fetching data:', error);
                showNotification(`Error: ${error.message}`, true);
                return null;
            }
        }
        
        // Render memory blocks visualization
        function renderMemoryBlocks(data) {
            if (!data || !data.blocks) return;
            
            memoryVisualization.innerHTML = '';
            
            // Calculate total memory size
            const totalMemory = data.blocks.reduce((sum, block) => sum + block.size, 0);
            totalMemorySizeDisplay.textContent = `Total Memory Size: ${totalMemory} KB`;
            
            // Find the largest block for scaling
            const maxBlockSize = Math.max(...data.blocks.map(block => block.size));
            
            data.blocks.forEach(block => {
                const blockElement = document.createElement('div');
                blockElement.className = 'memory-block';
                
                const blockId = document.createElement('div');
                blockId.className = 'block-id';
                blockId.textContent = block.id;
                blockElement.appendChild(blockId);
                
                const blockSize = document.createElement('div');
                blockSize.className = 'block-size';
                
                if (block.used) {
                    const usedWidth = (block.size / maxBlockSize) * 100;
                    const usedElement = document.createElement('div');
                    usedElement.className = 'block-used';
                    usedElement.style.width = `${usedWidth}%`;
                    
                    const processInfo = document.createElement('span');
                    processInfo.className = 'process-info';
                    processInfo.textContent = block.process;
                    usedElement.appendChild(processInfo);

                    const sizeText = document.createElement('span');
                    sizeText.className = 'block-size-text';
                    sizeText.textContent = `${block.size} KB`;
                    usedElement.appendChild(sizeText);
                    
                    blockSize.appendChild(usedElement);
                } else {
                    const freeWidth = (block.size / maxBlockSize) * 100;
                    const freeElement = document.createElement('div');
                    freeElement.className = 'block-free';
                    freeElement.style.width = `${freeWidth}%`;

                    const sizeText = document.createElement('span');
                    sizeText.className = 'block-size-text';
                    sizeText.textContent = `${block.size} KB`;
                    freeElement.appendChild(sizeText);

                    blockSize.appendChild(freeElement);
                }
                
                blockElement.appendChild(blockSize);
                memoryVisualization.appendChild(blockElement);
            });
        }
        
        // Render fragmentation stats
        function renderFragmentationStats(data) {
            if (!data) return;
            
            fragmentationStats.innerHTML = `
                <div class="stat-card">
                    <h3>Total Memory</h3>
                    <p>${data.total_memory} KB</p>
                </div>
                <div class="stat-card">
                    <h3>Used Memory</h3>
                    <p>${data.used_memory} KB</p>
                </div>
                <div class="stat-card">
                    <h3>Free Memory</h3>
                    <p>${data.free_memory} KB</p>
                </div>
                <div class="stat-card">
                    <h3>External Fragmentation</h3>
                    <p>${data.external_frag} KB</p>
                </div>
                <div class="stat-card">
                    <h3>Internal Fragmentation</h3>
                    <p>${data.internal_frag} KB</p>
                </div>
                <div class="stat-card">
                    <h3>Largest Free Block</h3>
                    <p>${data.largest_free_block} KB</p>
                </div>
                <div class="stat-card">
                    <h3>Fragmentation Percentage</h3>
                    <p>${data.fragmentation_percentage.toFixed(2)}%</p>
                </div>
            `;
        }
        
        // Render process list
        function renderProcessList(data) {
            if (!data || !data.processes) return;
            
            processList.innerHTML = '';
            
            if (data.processes.length === 0) {
                processList.innerHTML = '<p>No processes currently running</p>';
                return;
            }
            
            data.processes.forEach(process => {
                const processElement = document.createElement('div');
                processElement.className = 'process-item';
                
                processElement.innerHTML = `
                    <h4>${process.name}</h4>
                    <p><strong>Size:</strong> ${process.size} KB</p>
                    <p><strong>Block:</strong> ${process.block}</p>
                    <p><strong>Running Time:</strong> ${process.hours_running.toFixed(2)} hours</p>
                `;
                
                processList.appendChild(processElement);
            });
        }
        
        // Load all data
        async function loadAllData() {
            // Memory status
            const memoryData = await fetchData('/memory');
            renderMemoryBlocks(memoryData);
            
            // Fragmentation stats
            const fragData = await fetchData('/fragmentation');
            renderFragmentationStats(fragData);
            
            // Process list
            const processData = await fetchData('/processes');
            renderProcessList(processData);
        }
        
        // Event listeners
        document.getElementById('allocateBtn').addEventListener('click', async () => {
            const processName = document.getElementById('processName').value.trim();
            const processSize = parseInt(document.getElementById('processSize').value);
            const strategy = document.getElementById('strategy').value;
            
            if (!processName || isNaN(processSize) || processSize <= 0) {
                showNotification('Please enter valid process name and size', true);
                return;
            }
            
            const result = await fetchData(`/alloc?size=${processSize}&strategy=${strategy}&process=${processName}`);
            if (result) {
                showNotification(`Memory allocated in block ${result.block} (${result.size}KB)`);
                loadAllData();
            }
        });
        
        document.getElementById('deallocateBtn').addEventListener('click', async () => {
            const processName = document.getElementById('processName').value.trim();
            
            if (!processName) {
                showNotification('Please enter a process name to deallocate', true);
                return;
            }
            
            const result = await fetchData(`/dealloc?process=${processName}`);
            if (result) {
                showNotification(`Process ${processName} deallocated`);
                loadAllData();
            }
        });
        
        document.getElementById('resetBtn').addEventListener('click', async () => {
            const result = await fetchData('/reset');
            if (result) {
                showNotification('Memory reset to initial state');
                loadAllData();
            }
        });
        
        document.getElementById('compactBtn').addEventListener('click', async () => {
            const result = await fetchData('/compact');
            if (result) {
                showNotification('Memory compacted successfully');
                loadAllData();
            }
        });
        
        document.getElementById('refreshMemoryBtn').addEventListener('click', loadAllData);
        
        // Initial load
        loadAllData();
